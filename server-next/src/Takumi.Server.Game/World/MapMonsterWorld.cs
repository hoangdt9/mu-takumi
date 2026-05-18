using System.Globalization;
using Takumi.Server.Game.Networking;
using Takumi.Server.Persistence;

namespace Takumi.Server.Game.World;

/// <summary>M9A: static map monsters from set-base + stats (legacy <c>gMonsterSetBase</c> / viewport 0x13).</summary>
public static class MapMonsterWorld
{
    static readonly object InitLock = new();
    static bool _initialized;
    static IReadOnlyList<MonsterSetBaseEntry> _setBase = Array.Empty<MonsterSetBaseEntry>();
    static MonsterStatCatalog _stats = new();
    static Dictionary<byte, List<MapMonsterInstance>> _byMap = new();
    static Dictionary<int, MapMonsterInstance> _byObjectKey = new();
    static int _nextObjectKey = 12_000;

    public static bool TryGetMonster(int objectKey, out MapMonsterInstance? monster)
    {
        EnsureInitialized();
        return _byObjectKey.TryGetValue(objectKey, out monster);
    }

    public static IReadOnlyList<MapMonsterInstance> GetMonstersOnMap(byte mapId)
    {
        EnsureInitialized();
        return _byMap.TryGetValue(mapId, out var list) ? list : Array.Empty<MapMonsterInstance>();
    }

    public static IReadOnlyDictionary<byte, int> GetInstanceCountByMap()
    {
        EnsureInitialized();
        var result = new Dictionary<byte, int>(_byMap.Count);
        foreach (var (mapId, list) in _byMap)
        {
            result[mapId] = list.Count;
        }

        return result;
    }

    public static int GetNpcCountOnMap(byte mapId)
    {
        EnsureInitialized();
        if (!_byMap.TryGetValue(mapId, out var list))
        {
            return 0;
        }

        var count = 0;
        foreach (var m in list)
        {
            if (m.IsNpc)
            {
                count++;
            }
        }

        return count;
    }

    public static MonsterStat GetMonsterStat(int monsterClass)
    {
        EnsureInitialized();
        return _stats.GetOrDefault(monsterClass);
    }

    public static void EnsureInitialized()
    {
        if (_initialized)
        {
            return;
        }

        lock (InitLock)
        {
            if (_initialized)
            {
                return;
            }

            if (!TryLoadSetBaseFromPostgres(out var setPath))
            {
                setPath = ResolvePath(
                    "TAKUMI_MONSTER_SET_BASE_PATH",
                    "Monster/MonsterSetBase.txt");
                if (setPath is not null && File.Exists(setPath))
                {
                    _setBase = MonsterSetBaseLoader.LoadFromFile(setPath);
                    Console.WriteLine("[m9] loaded MonsterSetBase {0} entries from {1}", _setBase.Count, setPath);
                }
                else
                {
                    _setBase = BuildLorenciaFallback();
                    Console.WriteLine(
                        "[m9] MonsterSetBase missing ({0}) — using built-in Lorencia fallback ({1} rows: NPCs + field spots)",
                        setPath ?? "(unset)",
                        _setBase.Count);
                }
            }

            var monsterPath = ResolvePath(
                "TAKUMI_MONSTER_INFO_PATH",
                "Monster/Monster.txt");

            if (monsterPath is not null && File.Exists(monsterPath))
            {
                try
                {
                    _stats = MonsterStatCatalog.LoadFromFile(monsterPath);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("[m9] Monster.txt load failed ({0}) — using default stats", ex.Message);
                    _stats = new MonsterStatCatalog();
                }
            }

            var sw = System.Diagnostics.Stopwatch.StartNew();
            RebuildInstances();
            sw.Stop();
            var mapCount = _byMap.Count;
            var instCount = _byObjectKey.Count;
            Console.WriteLine(
                "[m9] MapMonsterWorld ready: {0} instances on {1} maps ({2} ms)",
                instCount,
                mapCount,
                sw.ElapsedMilliseconds);
            // Mark ready before coverage/ATT preload — LogStartupReport → BuildSummaryByMap
            // re-enters EnsureInitialized; C# lock is reentrant and would reload spawns forever.
            _initialized = true;
            MapAttWalkability.PreloadMaps(_byMap.Keys);
            MapMonsterSpawnCoverage.LogStartupReport();
        }
    }

    public static IReadOnlyList<MapMonsterInstance> GetMonstersNear(byte mapId, byte px, byte py, int viewRange, int maxCount) =>
        GetViewportEntities(mapId, px, py, viewRange, maxNpcs: maxCount, maxMonsters: maxCount);

    /// <summary>NPCs first (sorted by distance), then field monsters — parity town vendors visible on join.</summary>
    public static IReadOnlyList<MapMonsterInstance> GetViewportEntities(
        byte mapId,
        byte px,
        byte py,
        int viewRange,
        int maxNpcs,
        int maxMonsters)
    {
        EnsureInitialized();
        if (!_byMap.TryGetValue(mapId, out var all) || all.Count == 0)
        {
            return Array.Empty<MapMonsterInstance>();
        }

        var npcs = new List<(int Dist, MapMonsterInstance Mob)>();
        var mobs = new List<(int Dist, MapMonsterInstance Mob)>();
        foreach (var m in all)
        {
            if (!m.IsAlive)
            {
                _ = m.TryRegen();
                if (!m.IsAlive)
                {
                    continue;
                }
            }

            var dist = Math.Abs(m.X - px) + Math.Abs(m.Y - py);
            if (dist > viewRange)
            {
                continue;
            }

            if (m.IsNpc)
            {
                npcs.Add((dist, m));
            }
            else
            {
                mobs.Add((dist, m));
            }
        }

        npcs.Sort(static (a, b) => a.Dist.CompareTo(b.Dist));
        mobs.Sort(static (a, b) => a.Dist.CompareTo(b.Dist));

        var cap = Math.Max(1, maxNpcs + maxMonsters);
        var result = new List<MapMonsterInstance>(Math.Min(cap, npcs.Count + mobs.Count));
        foreach (var (_, m) in npcs)
        {
            if (result.Count >= maxNpcs)
            {
                break;
            }

            result.Add(m);
        }

        foreach (var (_, m) in mobs)
        {
            if (result.Count >= maxNpcs + maxMonsters)
            {
                break;
            }

            result.Add(m);
        }

        return result;
    }

    public static IReadOnlyList<MonsterAiEvent> ProcessAiTick(Random rng)
    {
        EnsureInitialized();
        var wanderPct = ParseIntEnv("TAKUMI_MONSTER_AI_WANDER_PCT", 28, 0, 100);
        var attackPct = ParseIntEnv("TAKUMI_MONSTER_AI_ATTACK_PCT", 12, 0, 100);
        var chaseRange = ParseIntEnv("TAKUMI_MONSTER_AI_CHASE_RANGE", 12, 3, 24);
        var attackRange = ParseIntEnv("TAKUMI_MONSTER_AI_ATTACK_RANGE", 3, 1, 8);
        var events = new List<MonsterAiEvent>();
        foreach (var m in _byObjectKey.Values)
        {
            if (!m.IsNpc && !m.IsAlive)
            {
                if (m.TryRegen())
                {
                    events.Add(
                        new MonsterAiEvent(
                            MonsterAiEventKind.Regen,
                            m.ObjectKey,
                            m.Map,
                            m.X,
                            m.Y,
                            m.Dir,
                            TargetObjectKey: 0));
                }

                continue;
            }

            if (m.IsNpc || !m.IsAlive)
            {
                continue;
            }

            if (MapAttWalkability.IsSafeZone(m.Map, m.X, m.Y))
            {
                if (rng.Next(100) < wanderPct && m.TryRollWander(rng, m.Map, out var szX, out var szY, out var szDir))
                {
                    events.Add(new MonsterAiEvent(MonsterAiEventKind.Walk, m.ObjectKey, m.Map, szX, szY, szDir, 0));
                }

                continue;
            }

            var stat = GetMonsterStat(m.MonsterClass);
            var mobAttackRange = Math.Max(attackRange, stat.AttackRange > 0 ? stat.AttackRange : attackRange);
            var mobChaseRange = Math.Max(chaseRange, stat.ViewRange > 0 ? stat.ViewRange : chaseRange);

            MonsterViewerTarget? target = null;
            if (MonsterViewerRegistry.TryFindNearestTarget(m.Map, m.X, m.Y, mobChaseRange, out var found)
                && !MapAttWalkability.IsSafeZone(m.Map, found.X, found.Y))
            {
                target = found;
                m.SetAggro(found.PlayerObjectKey);
            }
            else if (m.AggroTargetKey is not null)
            {
                m.ClearAggro();
            }

            if (target is { } t)
            {
                var dist = m.ManhattanTo(t.X, t.Y);
                if (dist > mobAttackRange)
                {
                    if (m.TryChaseStep(t.X, t.Y, m.Map, out var cx, out var cy, out var cdir))
                    {
                        events.Add(
                            new MonsterAiEvent(MonsterAiEventKind.Walk, m.ObjectKey, m.Map, cx, cy, cdir, t.PlayerObjectKey));
                    }

                    continue;
                }

                if (rng.Next(100) < attackPct)
                {
                    var useSkill = stat.UsesRangedOrMagic && rng.Next(100) < ParseIntEnv("TAKUMI_MONSTER_SKILL_CHANCE_PCT", 35, 0, 100);
                    events.Add(
                        new MonsterAiEvent(
                            useSkill ? MonsterAiEventKind.SkillAttack : MonsterAiEventKind.Attack,
                            m.ObjectKey,
                            m.Map,
                            m.X,
                            m.Y,
                            m.Dir,
                            t.PlayerObjectKey,
                            t.SessionId));
                }

                continue;
            }

            if (rng.Next(100) < wanderPct && m.TryRollWander(rng, m.Map, out var wx, out var wy, out var wdir))
            {
                events.Add(new MonsterAiEvent(MonsterAiEventKind.Walk, m.ObjectKey, m.Map, wx, wy, wdir, 0));
            }
        }

        return events;
    }

    static bool TryLoadSetBaseFromPostgres(out string? fileFallbackPath)
    {
        fileFallbackPath = null;
        var repo = TakumiPostgresMirror.MonsterSpawn;
        if (repo is null)
        {
            return false;
        }

        try
        {
            var rows = repo.LoadAllAsync().GetAwaiter().GetResult();
            if (rows.Count == 0)
            {
                Console.WriteLine("[m8] monster_spawn table empty — falling back to file");
                return false;
            }

            _setBase = rows.Select(MonsterSpawnRowMapping.FromRow).ToList();
            fileFallbackPath = null;
            Console.WriteLine("[m8] loaded {0} monster spawns from Postgres (monster_spawn)", _setBase.Count);
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine("[m8] Postgres monster_spawn load failed ({0}) — falling back to file", ex.Message);
            fileFallbackPath = ResolvePath(
                "TAKUMI_MONSTER_SET_BASE_PATH",
                "Monster/MonsterSetBase.txt");
            return false;
        }
    }

    static void RebuildInstances()
    {
        _byMap = new Dictionary<byte, List<MapMonsterInstance>>();
        _byObjectKey = new Dictionary<int, MapMonsterInstance>();
        var key = _nextObjectKey;
        foreach (var e in _setBase)
        {
            // Parity CMonsterManager::SetMonsterData — invasion/event rows are not static map spawns.
            if (!IncludeInvasionSpawns() && e.SpawnType is 3 or 4)
            {
                continue;
            }

            if (!MonsterSpawnResolver.TryResolvePosition(e, out var x, out var y))
            {
                continue;
            }

            var stat = _stats.GetOrDefault(e.MonsterClass);
            var isNpc = MonsterNpcClassifier.IsNpc(e.MonsterClass) || stat.Level == 0;
            var maxLife = isNpc ? Math.Max(1, stat.Life) : stat.Life;
            var regenMs = isNpc ? int.MaxValue / 2 : Math.Max(1, stat.RegenTimeSeconds) * 1000;
            var leash = (byte)Math.Clamp(e.Dis > 0 ? e.Dis : stat.MoveRange > 0 ? stat.MoveRange : 3, 1, 30);
            var moveRange = (byte)Math.Clamp(stat.MoveRange > 0 ? stat.MoveRange : 3, 1, 30);
            var inst = new MapMonsterInstance
            {
                ObjectKey = key++,
                MonsterClass = e.MonsterClass,
                Map = e.Map,
                SpawnX = x,
                SpawnY = y,
                WanderLeash = leash,
                MoveRange = moveRange,
                MaxLife = maxLife,
                Level = stat.Level,
                RegenDelayMs = regenMs,
                IsNpc = isNpc,
            };
            inst.InitializeAtSpawn(x, y, e.Dir);

            if (!_byMap.TryGetValue(inst.Map, out var bucket))
            {
                bucket = new List<MapMonsterInstance>();
                _byMap[inst.Map] = bucket;
            }

            bucket.Add(inst);
            _byObjectKey[inst.ObjectKey] = inst;
        }

        _nextObjectKey = key;
    }

    static string? ResolvePath(string envKey, string relativeDefault)
    {
        var env = Environment.GetEnvironmentVariable(envKey)?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        var candidates = new[]
        {
            Path.Combine(Environment.CurrentDirectory, relativeDefault),
            Path.Combine(Environment.CurrentDirectory, "..", "MuServer", "4.GameServer", "Data", relativeDefault),
            Path.Combine(Environment.CurrentDirectory, "..", "..", "MuServer", "4.GameServer", "Data", relativeDefault),
        };

        foreach (var c in candidates)
        {
            var full = Path.GetFullPath(c);
            if (File.Exists(full))
            {
                return full;
            }
        }

        return Path.GetFullPath(candidates[0]);
    }

    /// <summary>QA fallback when <c>MonsterSetBase.txt</c> is unavailable (Docker without Data mount).</summary>
    static IReadOnlyList<MonsterSetBaseEntry> BuildLorenciaFallback()
    {
        var list = new List<MonsterSetBaseEntry>(48);
        list.AddRange(BuildLorenciaNpcFallback());
        list.AddRange(BuildLorenciaFieldMonsterFallback());
        return list;
    }

    /// <summary>Parity Lorencia section 0 NPCs (MuServer <c>MonsterSetBase.txt</c>).</summary>
    static IEnumerable<MonsterSetBaseEntry> BuildLorenciaNpcFallback()
    {
        yield return Entry(0, 226, 122, 110, 3);
        yield return Entry(0, 230, 62, 130, 3);
        yield return Entry(0, 230, 118, 142, 2);
        yield return Entry(0, 379, 115, 139, 3);
        yield return Entry(0, 250, 183, 137, 2);
        yield return Entry(0, 253, 127, 86, 2);
        yield return Entry(0, 251, 116, 141, 3);
        yield return Entry(0, 255, 122, 135, 1);
        yield return Entry(0, 244, 126, 135, 1);
        yield return Entry(0, 254, 118, 113, 3);
        yield return Entry(0, 240, 147, 145, 1);
        yield return Entry(0, 240, 146, 110, 3);
        yield return Entry(0, 257, 96, 129, 1);
        yield return Entry(0, 257, 132, 165, 3);
        yield return Entry(0, 257, 132, 90, 3);
        yield return Entry(0, 257, 170, 129, 1);
        yield return Entry(0, 479, 130, 133, 3);
        yield return Entry(0, 246, 120, 142, 2);
    }

    /// <summary>Parity Lorencia section 1 field spots (outside safe zone).</summary>
    static IEnumerable<MonsterSetBaseEntry> BuildLorenciaFieldMonsterFallback()
    {
        yield return Spot(2, 187, 117, 187, 117);
        yield return Spot(3, 187, 123, 187, 123);
        yield return Spot(0, 134, 82, 134, 82);
        yield return Spot(0, 141, 82, 141, 82);
        yield return Spot(4, 86, 116, 86, 116);
        yield return Spot(4, 86, 137, 86, 137);
    }

    static MonsterSetBaseEntry Entry(int spawnType, int monsterClass, int x, int y, byte dir) =>
        new()
        {
            SpawnType = spawnType,
            MonsterClass = monsterClass,
            Map = 0,
            Dis = 0,
            X = x,
            Y = y,
            Dir = dir,
        };

    static MonsterSetBaseEntry Spot(int monsterClass, int x, int y, int tx, int ty) =>
        new()
        {
            SpawnType = 1,
            MonsterClass = monsterClass,
            Map = 0,
            Dis = 5,
            X = x,
            Y = y,
            Tx = tx,
            Ty = ty,
            Dir = 255,
        };

    public static bool ShouldIncludeSpawnEntry(MonsterSetBaseEntry e) =>
        IncludeInvasionSpawns() || e.SpawnType is not (3 or 4);

    static bool IncludeInvasionSpawns() =>
        string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_MONSTER_INCLUDE_INVASION_SPAWN")?.Trim(),
            "1",
            StringComparison.OrdinalIgnoreCase);

    static int ParseIntEnv(string key, int defaultValue, int min, int max)
    {
        var raw = Environment.GetEnvironmentVariable(key);
        if (string.IsNullOrWhiteSpace(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var v))
        {
            return defaultValue;
        }

        return Math.Clamp(v, min, max);
    }
}
