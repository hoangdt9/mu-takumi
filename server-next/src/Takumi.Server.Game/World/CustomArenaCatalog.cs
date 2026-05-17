namespace Takumi.Server.Game.World;

/// <summary>M8: custom arena warp gates (<c>CA_MAP_RANGE</c> / <c>CheckEnterEnabled</c>).</summary>
public static class CustomArenaCatalog
{
    static readonly object InitLock = new();
    static bool _initialized;
    static Dictionary<int, CustomArenaRuleEntry> _byStartGate = new();
    static Dictionary<int, List<CustomArenaStartTimeEntry>> _schedulesByArena = new();

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

            var path = ResolvePath();
            if (path is not null && File.Exists(path))
            {
                var loaded = CustomArenaLoader.LoadFromFile(path);
                Rebuild(loaded.Rules);
                RebuildSchedules(loaded.Schedules);
                Console.WriteLine(
                    "[m8] CustomArenaCatalog: {0} rules, {1} schedule rows from {2}",
                    _byStartGate.Count,
                    loaded.Schedules.Count,
                    path);
            }
            else
            {
                _byStartGate = new Dictionary<int, CustomArenaRuleEntry>();
                _schedulesByArena = new Dictionary<int, List<CustomArenaStartTimeEntry>>();
                Console.WriteLine("[m8] CustomArenaCatalog: no CustomArena.txt ({0})", path ?? "(unset)");
            }

            _initialized = true;
            CustomArenaScheduleFsm.RebuildRuntimes();
        }
    }

    public static bool TryGetByStartGate(int gate, out CustomArenaRuleEntry? rule)
    {
        EnsureInitialized();
        if (_byStartGate.TryGetValue(gate, out var row))
        {
            rule = row;
            return true;
        }

        rule = null;
        return false;
    }

    public static IReadOnlyList<CustomArenaRuleEntry> GetAllRules()
    {
        EnsureInitialized();
        return _byStartGate.Values.ToList();
    }

    public static IReadOnlyList<CustomArenaStartTimeEntry> GetSchedules(int arenaIndex)
    {
        EnsureInitialized();
        if (_schedulesByArena.TryGetValue(arenaIndex, out var list))
        {
            return list;
        }

        return Array.Empty<CustomArenaStartTimeEntry>();
    }

    static void Rebuild(IReadOnlyList<CustomArenaRuleEntry> rows)
    {
        _byStartGate = new Dictionary<int, CustomArenaRuleEntry>(rows.Count);
        foreach (var row in rows)
        {
            _byStartGate[row.StartGate] = row;
        }
    }

    static void RebuildSchedules(IReadOnlyList<CustomArenaStartTimeEntry> rows)
    {
        _schedulesByArena = new Dictionary<int, List<CustomArenaStartTimeEntry>>();
        foreach (var row in rows)
        {
            if (!_schedulesByArena.TryGetValue(row.ArenaIndex, out var list))
            {
                list = new List<CustomArenaStartTimeEntry>();
                _schedulesByArena[row.ArenaIndex] = list;
            }

            list.Add(row);
        }
    }

    public static void LoadForTests(
        IReadOnlyList<CustomArenaRuleEntry> rules,
        IReadOnlyList<CustomArenaStartTimeEntry>? schedules = null)
    {
        lock (InitLock)
        {
            Rebuild(rules);
            RebuildSchedules(schedules ?? Array.Empty<CustomArenaStartTimeEntry>());
            _initialized = true;
            CustomArenaScheduleFsm.RebuildRuntimes();
        }
    }

    static string? ResolvePath()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_CUSTOM_ARENA_PATH")?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        var root = WorldDataPathResolver.ResolveDataRoot();
        return root is null ? null : Path.Combine(root, "Custom", "CustomArena.txt");
    }

    /// <summary>When 1, skip live arena schedule (<c>EnterEnabled</c> FSM) — rules-only check.</summary>
    public static bool SkipScheduleCheck() =>
        !string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_CUSTOM_ARENA_SKIP_SCHEDULE"),
            "0",
            StringComparison.OrdinalIgnoreCase);
}
