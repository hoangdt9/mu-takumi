namespace Takumi.Server.Game.World;

/// <summary>M8: map flags from <c>MapManager.txt</c> (<c>GetMapGensBattle</c>).</summary>
public static class MapManagerCatalog
{
    public const byte MapSilent = 40;

    static readonly object InitLock = new();
    static bool _initialized;
    static Dictionary<byte, MapManagerEntry> _byMap = new();

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
                var rows = MapManagerLoader.LoadFromFile(path);
                Rebuild(rows);
                Console.WriteLine("[m8] MapManagerCatalog: {0} maps from {1}", _byMap.Count, path);
            }
            else
            {
                _byMap = new Dictionary<byte, MapManagerEntry>();
                Console.WriteLine("[m8] MapManagerCatalog: no MapManager.txt ({0})", path ?? "(unset)");
            }

            _initialized = true;
        }
    }

    public static bool IsGensBattleMap(byte mapId)
    {
        EnsureInitialized();
        return _byMap.TryGetValue(mapId, out var row) && row.GensBattle != 0;
    }

    public static bool IsCustomArenaMap(byte mapId) => mapId == MapSilent;

    static void Rebuild(IReadOnlyList<MapManagerEntry> rows)
    {
        _byMap = new Dictionary<byte, MapManagerEntry>(rows.Count);
        foreach (var row in rows)
        {
            _byMap[row.MapId] = row;
        }
    }

    public static void LoadForTests(IReadOnlyList<MapManagerEntry> rows)
    {
        lock (InitLock)
        {
            Rebuild(rows);
            _initialized = true;
        }
    }

    static string? ResolvePath()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_MAP_MANAGER_PATH")?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        var root = WorldDataPathResolver.ResolveDataRoot();
        return root is null ? null : Path.Combine(root, "MapManager.txt");
    }
}
