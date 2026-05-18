namespace Takumi.Server.Protocol;

/// <summary>Maps item index → set option index (parity <c>SetItemType.txt</c>).</summary>
public static class SetItemTypeCatalog
{
    public readonly record struct SetItemTypeRow(int ItemIndex, int StatType, int OptionIndex0, int OptionIndex1);

    static readonly object Gate = new();
    static bool _ready;
    static Dictionary<int, SetItemTypeRow> _byItem = new();

    public static void EnsureInitialized()
    {
        if (_ready)
        {
            return;
        }

        lock (Gate)
        {
            if (_ready)
            {
                return;
            }

            var path = ResolvePath();
            if (path is not null)
            {
                Load(path);
            }

            _ready = true;
        }
    }

    public static bool TryGet(int itemIndex, out SetItemTypeRow row)
    {
        EnsureInitialized();
        return _byItem.TryGetValue(itemIndex, out row);
    }

    public static int GetSetOptionIndex(int itemIndex, int ancientTier)
    {
        if (!TryGet(itemIndex, out var row))
        {
            return 0;
        }

        return ancientTier switch
        {
            0 => row.OptionIndex0,
            1 => row.OptionIndex1,
            _ => 0,
        };
    }

    public static int GetStatType(int itemIndex) =>
        TryGet(itemIndex, out var row) ? row.StatType : -1;

    public static string? ResolvePath()
    {
        var explicitPath = Environment.GetEnvironmentVariable("TAKUMI_SET_ITEM_TYPE_PATH")?.Trim();
        if (!string.IsNullOrEmpty(explicitPath) && File.Exists(explicitPath))
        {
            return Path.GetFullPath(explicitPath);
        }

        return ResolveUnderData("SetItemType.txt");
    }

    internal static string? ResolveUnderData(string fileName)
    {
        var candidates = new[]
        {
            Path.Combine(Environment.CurrentDirectory, "Data", "Item", fileName),
            Path.Combine(AppContext.BaseDirectory, "Data", "Item", fileName),
            Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "..", "MuServer", "4.GameServer", "Data", "Item", fileName),
            Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "..", "..", "MuServer", "4.GameServer", "Data", "Item", fileName),
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

    static void Load(string path)
    {
        var map = new Dictionary<int, SetItemTypeRow>();
        foreach (var line in File.ReadLines(path))
        {
            var t = line.Trim();
            if (t.Length == 0 || t.StartsWith("//", StringComparison.Ordinal))
            {
                continue;
            }

            var parts = t.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length < 5)
            {
                continue;
            }

            if (!int.TryParse(parts[0], out var section) || !int.TryParse(parts[1], out var number))
            {
                continue;
            }

            if (!int.TryParse(parts[2], out var statType)
                || !int.TryParse(parts[3], out var opt0)
                || !int.TryParse(parts[4], out var opt1))
            {
                continue;
            }

            var itemIndex = section * 512 + number;
            map[itemIndex] = new SetItemTypeRow(itemIndex, statType, opt0, opt1);
        }

        _byItem = map;
    }
}
