namespace Takumi.Server.Protocol;

/// <summary>Loads GameServer <c>ItemOption.txt</c> and resolves per-item special options (parity <c>CItemOption::GetItemOption</c>).</summary>
public static class ItemOptionCatalog
{
    public readonly record struct ItemOptionRow(
        int SpecialIndex,
        int OptionIndex,
        int OptionValue,
        int ItemMinIndex,
        int ItemMaxIndex,
        int ItemOption1,
        int ItemOption2,
        int ItemOption3,
        int ItemNewOption);

    static readonly object Gate = new();
    static bool _ready;
    static Dictionary<int, List<ItemOptionRow>> _bySpecial = new();

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

    public static IReadOnlyList<(int OptionIndex, int OptionValue)> ResolveOptions(ReadOnlySpan<byte> item12)
    {
        EnsureInitialized();
        if (ItemWire602.IsEmpty(item12))
        {
            return Array.Empty<(int, int)>();
        }

        var index = ItemWire602.DecodeItemIndex(item12);
        if (index < 0)
        {
            return Array.Empty<(int, int)>();
        }

        var option1 = ItemWireDecode602.DecodeOption1Skill(item12);
        var option2 = ItemWireDecode602.DecodeOption2Luck(item12);
        var option3 = ItemWireDecode602.DecodeOption3(item12);
        var newOption = ItemWireDecode602.DecodeNewOption(item12);

        var list = new List<(int, int)>(8);
        for (var special = 0; special < 8; special++)
        {
            if (!_bySpecial.TryGetValue(special, out var rows))
            {
                continue;
            }

            foreach (var row in rows)
            {
                if (!Matches(row, index, option1, option2, option3, newOption))
                {
                    continue;
                }

                list.Add((row.OptionIndex, row.OptionValue));
                break;
            }
        }

        return list;
    }

    static bool Matches(ItemOptionRow row, int index, int option1, int option2, int option3, int newOption)
    {
        if (row.ItemMinIndex >= 0 && index < row.ItemMinIndex)
        {
            return false;
        }

        if (row.ItemMaxIndex >= 0 && index > row.ItemMaxIndex)
        {
            return false;
        }

        if (row.ItemOption1 >= 0 && option1 < row.ItemOption1)
        {
            return false;
        }

        if (row.ItemOption2 >= 0 && option2 < row.ItemOption2)
        {
            return false;
        }

        if (row.ItemOption3 >= 0 && option3 < row.ItemOption3)
        {
            return false;
        }

        if (row.ItemNewOption >= 0 && (newOption & row.ItemNewOption) == 0)
        {
            return false;
        }

        return true;
    }

    public static string? ResolvePath()
    {
        var explicitPath = Environment.GetEnvironmentVariable("TAKUMI_ITEM_OPTION_PATH")?.Trim();
        if (!string.IsNullOrEmpty(explicitPath) && File.Exists(explicitPath))
        {
            return Path.GetFullPath(explicitPath);
        }

        var candidates = new[]
        {
            Path.Combine(Environment.CurrentDirectory, "Data", "Item", "ItemOption.txt"),
            Path.Combine(AppContext.BaseDirectory, "Data", "Item", "ItemOption.txt"),
            Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "..", "MuServer", "4.GameServer", "Data", "Item", "ItemOption.txt"),
            Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "..", "..", "MuServer", "4.GameServer", "Data", "Item", "ItemOption.txt"),
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
        var map = new Dictionary<int, List<ItemOptionRow>>();
        foreach (var line in File.ReadLines(path))
        {
            var t = line.Trim();
            if (t.Length == 0 || t.StartsWith("//", StringComparison.Ordinal))
            {
                continue;
            }

            var parts = t.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length < 9)
            {
                continue;
            }

            if (!int.TryParse(parts[0], out var special))
            {
                continue;
            }

            if (!int.TryParse(parts[1], out var optionIndex)
                || !int.TryParse(parts[2], out var optionValue)
                || !TryParseIndex(parts[3], out var minIndex)
                || !TryParseIndex(parts[4], out var maxIndex)
                || !TryParseReq(parts[5], out var req1)
                || !TryParseReq(parts[6], out var req2)
                || !TryParseReq(parts[7], out var req3)
                || !TryParseReq(parts[8], out var reqNew))
            {
                continue;
            }

            if (!map.TryGetValue(special, out var rows))
            {
                rows = [];
                map[special] = rows;
            }

            rows.Add(new ItemOptionRow(
                special,
                optionIndex,
                optionValue,
                minIndex,
                maxIndex,
                req1,
                req2,
                req3,
                reqNew));
        }

        _bySpecial = map;
    }

    static bool TryParseIndex(string s, out int value)
    {
        if (s == "*")
        {
            value = -1;
            return true;
        }

        return int.TryParse(s, out value);
    }

    static bool TryParseReq(string s, out int value)
    {
        if (s == "*")
        {
            value = -1;
            return true;
        }

        return int.TryParse(s, out value);
    }
}
