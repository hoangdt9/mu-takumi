using System.Globalization;

namespace Takumi.Server.Game.World;

/// <summary>Socket-capable items from <c>Item/SocketItemType.txt</c> (parity <c>gSocketItemType</c>).</summary>
public static class SocketItemTypeCatalog
{
    sealed record SocketTypeRow(int MaxSocket);

    static readonly object Gate = new();
    static Dictionary<int, SocketTypeRow> _byItemIndex = new();

    public const byte NoSocket = 0xFF;

    public static void EnsureInitialized()
    {
        if (_byItemIndex.Count > 0)
        {
            return;
        }

        lock (Gate)
        {
            if (_byItemIndex.Count > 0)
            {
                return;
            }

            var path = ResolvePath();
            if (path is not null && File.Exists(path))
            {
                _byItemIndex = LoadFromFile(path);
                Console.WriteLine("[m8] SocketItemTypeCatalog: {0} types from {1}", _byItemIndex.Count, path);
            }
            else
            {
                _byItemIndex = new Dictionary<int, SocketTypeRow>();
                Console.WriteLine("[m8] SocketItemTypeCatalog: no SocketItemType.txt ({0})", path ?? "(unset)");
            }
        }
    }

    public static bool IsSocketItem(int itemIndex, out int maxSocket)
    {
        EnsureInitialized();
        if (_byItemIndex.TryGetValue(itemIndex, out var row))
        {
            maxSocket = row.MaxSocket;
            return maxSocket > 0;
        }

        maxSocket = 0;
        return false;
    }

    public static bool IsSocketItem(int itemGroup, int itemIndex, out int maxSocket) =>
        IsSocketItem((itemGroup * 512) + itemIndex, out maxSocket);

    public static void LoadForTests(IReadOnlyDictionary<int, int> maxSocketByIndex)
    {
        lock (Gate)
        {
            _byItemIndex = maxSocketByIndex.ToDictionary(kv => kv.Key, kv => new SocketTypeRow(kv.Value));
        }
    }

    static Dictionary<int, SocketTypeRow> LoadFromFile(string path)
    {
        var map = new Dictionary<int, SocketTypeRow>();
        foreach (var parts in GameDataTextTableLoader.ReadDataRows(path))
        {
            if (parts.Length < 3)
            {
                continue;
            }

            var section = GameDataTextTableLoader.ParseInt(parts[0]);
            var type = GameDataTextTableLoader.ParseInt(parts[1]);
            var maxSocket = GameDataTextTableLoader.ParseInt(parts[2]);
            if (maxSocket <= 0)
            {
                continue;
            }

            map[(section * 512) + type] = new SocketTypeRow(maxSocket);
        }

        return map;
    }

    static string? ResolvePath()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_SOCKET_ITEM_TYPE_PATH")?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        var dataRoot = Environment.GetEnvironmentVariable("TAKUMI_GAMESERVER_DATA_PATH")?.Trim();
        if (!string.IsNullOrEmpty(dataRoot))
        {
            return Path.Combine(Path.GetFullPath(dataRoot), "Item", "SocketItemType.txt");
        }

        var candidates = new[]
        {
            Path.Combine(Environment.CurrentDirectory, "Item", "SocketItemType.txt"),
            Path.Combine(Environment.CurrentDirectory, "..", "MuServer", "4.GameServer", "Data", "Item", "SocketItemType.txt"),
            Path.Combine(Environment.CurrentDirectory, "..", "..", "MuServer", "4.GameServer", "Data", "Item", "SocketItemType.txt"),
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
