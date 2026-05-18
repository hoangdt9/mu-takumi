namespace Takumi.Server.Game.World;

/// <summary>M8: warp list entries for move-map UI (<c>0x8E 02</c>).</summary>
public static class MoveMapCatalog
{
    static readonly object InitLock = new();
    static bool _initialized;
    static Dictionary<int, MoveMapEntry> _byIndex = new();

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

            var path = ResolveMovePath();
            if (path is not null && File.Exists(path))
            {
                var rows = MoveLoader.LoadFromFile(path);
                Rebuild(rows);
                Console.WriteLine("[m8] MoveMapCatalog: {0} moves from {1}", _byIndex.Count, path);
            }
            else
            {
                _byIndex = new Dictionary<int, MoveMapEntry>();
                Console.WriteLine("[m8] MoveMapCatalog: no Move.txt ({0})", path ?? "(unset)");
            }

            _initialized = true;
        }
    }

    static void Rebuild(IReadOnlyList<MoveMapEntry> rows)
    {
        _byIndex = new Dictionary<int, MoveMapEntry>(rows.Count);
        var duplicateKeys = 0;
        foreach (var row in rows)
        {
            if (_byIndex.ContainsKey(row.Index))
            {
                duplicateKeys++;
            }

            _byIndex[row.Index] = row;
        }

        if (duplicateKeys > 0)
        {
            Console.WriteLine(
                "[m8] MoveMapCatalog: {0} duplicate Index row(s) in source — last row wins (parity custom Move.txt)",
                duplicateKeys);
        }
    }

    public static bool TryGet(int moveIndex, out MoveMapEntry? entry)
    {
        EnsureInitialized();
        if (_byIndex.TryGetValue(moveIndex, out var row))
        {
            entry = row;
            return true;
        }

        entry = null;
        return false;
    }

    /// <summary>All move rows (coverage / diagnostics).</summary>
    public static bool TryGetAllForCoverage(out IReadOnlyList<MoveMapEntry> moves)
    {
        EnsureInitialized();
        if (_byIndex.Count == 0)
        {
            moves = Array.Empty<MoveMapEntry>();
            return false;
        }

        moves = _byIndex.Values.ToList();
        return true;
    }

    /// <summary>Replaces catalog contents (unit tests only).</summary>
    public static void LoadForTests(IReadOnlyList<MoveMapEntry> rows)
    {
        lock (InitLock)
        {
            Rebuild(rows);
            _initialized = true;
        }
    }

    static string? ResolveMovePath()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_MOVE_PATH")?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        var root = WorldDataPathResolver.ResolveDataRoot();
        return root is null ? null : Path.Combine(root, "Move", "Move.txt");
    }
}
