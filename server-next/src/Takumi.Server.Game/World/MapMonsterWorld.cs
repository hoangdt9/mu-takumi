using System.Globalization;
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
    static int _nextObjectKey = 12_000;

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
                        "[m9] MonsterSetBase missing ({0}) — using built-in Lorencia fallback ({1} spawns)",
                        setPath ?? "(unset)",
                        _setBase.Count);
                }
            }

            var monsterPath = ResolvePath(
                "TAKUMI_MONSTER_INFO_PATH",
                "Monster/Monster.txt");

            if (monsterPath is not null && File.Exists(monsterPath))
            {
                _stats = MonsterStatCatalog.LoadFromFile(monsterPath);
                Console.WriteLine("[m9] loaded Monster.txt from {0}", monsterPath);
            }

            RebuildInstances();
            _initialized = true;
        }
    }

    public static IReadOnlyList<MapMonsterInstance> GetMonstersNear(byte mapId, byte px, byte py, int viewRange, int maxCount)
    {
        EnsureInitialized();
        if (!_byMap.TryGetValue(mapId, out var all) || all.Count == 0)
        {
            return Array.Empty<MapMonsterInstance>();
        }

        var list = new List<MapMonsterInstance>(Math.Min(maxCount, all.Count));
        foreach (var m in all)
        {
            if (Math.Abs(m.X - px) + Math.Abs(m.Y - py) > viewRange)
            {
                continue;
            }

            list.Add(m);
            if (list.Count >= maxCount)
            {
                break;
            }
        }

        return list;
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
        var key = _nextObjectKey;
        foreach (var e in _setBase)
        {
            if (!MonsterSpawnResolver.TryResolvePosition(e, out var x, out var y))
            {
                continue;
            }

            var stat = _stats.GetOrDefault(e.MonsterClass);
            var inst = new MapMonsterInstance
            {
                ObjectKey = key++,
                MonsterClass = e.MonsterClass,
                Map = e.Map,
                X = x,
                Y = y,
                Dir = e.Dir,
                Life = stat.Life,
                Level = stat.Level,
            };

            if (!_byMap.TryGetValue(inst.Map, out var bucket))
            {
                bucket = new List<MapMonsterInstance>();
                _byMap[inst.Map] = bucket;
            }

            bucket.Add(inst);
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

    static IReadOnlyList<MonsterSetBaseEntry> BuildLorenciaFallback()
    {
        return new List<MonsterSetBaseEntry>
        {
            new() { SpawnType = 0, MonsterClass = 3, Map = 0, Dis = 0, X = 180, Y = 120, Dir = 3 },
            new() { SpawnType = 0, MonsterClass = 2, Map = 0, Dis = 0, X = 140, Y = 90, Dir = 1 },
            new() { SpawnType = 0, MonsterClass = 0, Map = 0, Dis = 0, X = 130, Y = 130, Dir = 5 },
            new() { SpawnType = 0, MonsterClass = 257, Map = 0, Dis = 0, X = 130, Y = 118, Dir = 3 },
        };
    }
}
