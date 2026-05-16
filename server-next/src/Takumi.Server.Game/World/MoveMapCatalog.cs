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
                _byIndex = rows.ToDictionary(r => r.Index);
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

    /// <summary>Replaces catalog contents (unit tests only).</summary>
    public static void LoadForTests(IReadOnlyList<MoveMapEntry> rows)
    {
        lock (InitLock)
        {
            _byIndex = rows.ToDictionary(r => r.Index);
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
