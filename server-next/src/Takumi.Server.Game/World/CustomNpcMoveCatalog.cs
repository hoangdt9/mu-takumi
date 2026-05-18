namespace Takumi.Server.Game.World;

/// <summary>NPC-position warps from <c>Custom/CustomNpcMove.txt</c>.</summary>
public static class CustomNpcMoveCatalog
{
    static readonly object InitLock = new();
    static bool _initialized;
    static Dictionary<(int Class, byte Map, byte X, byte Y), CustomNpcMoveEntry> _byNpc = new();

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
                var rows = CustomNpcMoveLoader.LoadFromFile(path);
                Rebuild(rows);
                Console.WriteLine("[m8] CustomNpcMoveCatalog: {0} rows from {1}", _byNpc.Count, path);
            }
            else
            {
                _byNpc = new Dictionary<(int, byte, byte, byte), CustomNpcMoveEntry>();
                Console.WriteLine("[m8] CustomNpcMoveCatalog: no CustomNpcMove.txt ({0})", path ?? "(unset)");
            }

            _initialized = true;
        }
    }

    public static bool TryGetByNpc(int monsterClass, byte map, byte x, byte y, out CustomNpcMoveEntry? entry)
    {
        EnsureInitialized();
        if (_byNpc.TryGetValue((monsterClass, map, x, y), out var row))
        {
            entry = row;
            return true;
        }

        entry = null;
        return false;
    }

    public static void LoadForTests(IReadOnlyList<CustomNpcMoveEntry> rows)
    {
        lock (InitLock)
        {
            Rebuild(rows);
            _initialized = true;
        }
    }

    static void Rebuild(IReadOnlyList<CustomNpcMoveEntry> rows)
    {
        _byNpc = new Dictionary<(int, byte, byte, byte), CustomNpcMoveEntry>(rows.Count);
        foreach (var row in rows)
        {
            _byNpc[(row.MonsterClass, row.NpcMap, row.NpcX, row.NpcY)] = row;
        }
    }

    static string? ResolvePath()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_CUSTOM_NPC_MOVE_PATH")?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        var dataRoot = Environment.GetEnvironmentVariable("TAKUMI_GAMESERVER_DATA_PATH")?.Trim();
        if (!string.IsNullOrEmpty(dataRoot))
        {
            return Path.Combine(Path.GetFullPath(dataRoot), "Custom", "CustomNpcMove.txt");
        }

        var candidates = new[]
        {
            Path.Combine(Environment.CurrentDirectory, "Custom", "CustomNpcMove.txt"),
            Path.Combine(Environment.CurrentDirectory, "..", "MuServer", "4.GameServer", "Data", "Custom", "CustomNpcMove.txt"),
            Path.Combine(Environment.CurrentDirectory, "..", "..", "MuServer", "4.GameServer", "Data", "Custom", "CustomNpcMove.txt"),
        };

        foreach (var c in candidates)
        {
            var full = Path.GetFullPath(c);
            if (File.Exists(full))
            {
                return full;
            }
        }

        return null;
    }
}
