using System.Globalization;

namespace Takumi.Server.Game.World;

/// <summary>Loads <c>Item/ItemValue.txt</c> (parity <c>CItemValue::Load</c>).</summary>
public static class ItemValueCatalog
{
    sealed record ItemValueRow(int Index, int Level, int Grade, int Value, int Coin1, int Coin2, int Coin3, int Sell);

    static readonly object InitLock = new();
    static bool _initialized;
    static List<ItemValueRow> _rows = [];

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

            _rows = LoadFromFile() ?? [];
            _initialized = true;
            Console.WriteLine("[m8] ItemValueCatalog: {0} rows", _rows.Count);
        }
    }

    public static bool TryGetBuySell(
        int itemIndex,
        int level,
        int excellent,
        out int buyValue,
        out int sellValue)
    {
        EnsureInitialized();
        buyValue = 0;
        sellValue = 0;
        if (!TryMatchRow(itemIndex, level, excellent, out var row) || row is null)
        {
            return false;
        }

        buyValue = row.Value;
        sellValue = row.Sell > 0 ? row.Sell : Math.Max(1, row.Value / 3);
        return true;
    }

    /// <summary>Exact <c>Level</c> + <c>OpExe</c> row only (no <c>*</c> grade wildcard).</summary>
    public static bool TryGetBuySellExact(
        int itemIndex,
        int level,
        int excellent,
        out int buyValue,
        out int sellValue)
    {
        EnsureInitialized();
        buyValue = 0;
        sellValue = 0;
        if (!TryMatchRowExact(itemIndex, level, excellent, out var row) || row is null)
        {
            return false;
        }

        buyValue = row.Value;
        sellValue = row.Sell > 0 ? row.Sell : Math.Max(1, row.Value / 3);
        return true;
    }

    /// <summary>Wire row for <c>GCItemValueSend</c> (parity <c>CShop::GCItemValueSend</c> coin type fields).</summary>
    public static bool TryGetWirePrice(
        int itemIndex,
        int level,
        int excellent,
        out int priceType,
        out int value,
        out int sellValue)
    {
        EnsureInitialized();
        priceType = 0;
        value = 0;
        sellValue = 0;
        if (!TryMatchRow(itemIndex, level, excellent, out var row) || row is null)
        {
            return false;
        }

        priceType = 0;
        value = row.Value;
        sellValue = row.Sell > 0 ? row.Sell : Math.Max(1, row.Value / 3);
        if (row.Coin3 > 0)
        {
            priceType = 3;
            value = row.Coin3;
        }
        else if (row.Coin2 > 0)
        {
            priceType = 2;
            value = row.Coin2;
        }
        else if (row.Coin1 > 0)
        {
            priceType = 1;
            value = row.Coin1;
        }

        return true;
    }

    static bool TryMatchRowExact(int itemIndex, int level, int excellent, out ItemValueRow? row)
    {
        row = null;
        foreach (var candidate in _rows)
        {
            if (candidate.Index == itemIndex && candidate.Level == level && candidate.Grade == excellent)
            {
                row = candidate;
                return true;
            }
        }

        return false;
    }

    static bool TryMatchRow(int itemIndex, int level, int excellent, out ItemValueRow? row)
    {
        row = null;
        ItemValueRow? best = null;
        foreach (var candidate in _rows)
        {
            if (candidate.Index != itemIndex)
            {
                continue;
            }

            if (candidate.Level == level && candidate.Grade == excellent)
            {
                row = candidate;
                return true;
            }

            if (candidate.Level == level && candidate.Grade == -1)
            {
                best ??= candidate;
            }
            else if (candidate.Level == -1 && candidate.Grade == excellent)
            {
                best ??= candidate;
            }
            else if (candidate.Level == -1 || candidate.Level == level)
            {
                if (candidate.Grade == -1 || candidate.Grade == excellent)
                {
                    best ??= candidate;
                }
            }
        }

        if (best is not null)
        {
            row = best;
            return true;
        }

        return false;
    }

    /// <summary>True when <c>ItemValue.txt</c> row uses coin columns with zero zen <c>Value</c> (legacy cash shop).</summary>
    public static bool IsCoinOnlyPrice(int itemIndex, int level, int excellent)
    {
        EnsureInitialized();
        foreach (var row in _rows)
        {
            if (row.Index != itemIndex)
            {
                continue;
            }

            if (row.Level != -1 && row.Level != level)
            {
                continue;
            }

            if (row.Grade != -1 && row.Grade != excellent)
            {
                continue;
            }

            var anyCoin = row.Coin1 != 0 || row.Coin2 != 0 || row.Coin3 != 0;
            return anyCoin && row.Value <= 0;
        }

        return false;
    }

    static List<ItemValueRow>? LoadFromFile()
    {
        var path = ResolvePath();
        if (path is null || !File.Exists(path))
        {
            return null;
        }

        var rows = new List<ItemValueRow>();
        foreach (var line in File.ReadLines(path))
        {
            var t = line.Trim();
            if (t.Length == 0 || t.StartsWith("//", StringComparison.Ordinal) || t.Equals("end", StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            var parts = t.Split([' ', '\t'], StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length < 8)
            {
                continue;
            }

            if (!int.TryParse(parts[0], NumberStyles.Integer, CultureInfo.InvariantCulture, out var index))
            {
                continue;
            }

            rows.Add(
                new ItemValueRow(
                    index,
                    ParseLevelOrGrade(parts[1]),
                    ParseLevelOrGrade(parts[2]),
                    ParseInt(parts[3]),
                    ParseInt(parts[4]),
                    ParseInt(parts[5]),
                    ParseInt(parts[6]),
                    ParseInt(parts[7])));
        }

        return rows;
    }

    static int ParseInt(string s) =>
        int.TryParse(s, NumberStyles.Integer, CultureInfo.InvariantCulture, out var v) ? v : 0;

    static int ParseLevelOrGrade(string s) =>
        s is "*" or "-1" ? -1 : ParseInt(s);

    static string? ResolvePath()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_ITEM_VALUE_PATH")?.Trim();
        if (!string.IsNullOrEmpty(env) && File.Exists(env))
        {
            return env;
        }

        var roots = new[]
        {
            Environment.GetEnvironmentVariable("TAKUMI_GAMESERVER_DATA_PATH")?.Trim(),
            Path.Combine(Environment.CurrentDirectory, "Data"),
            Path.Combine(Environment.CurrentDirectory, "..", "MuServer", "4.GameServer", "Data"),
            Path.Combine(Environment.CurrentDirectory, "..", "MuServer", "Data"),
        };

        foreach (var root in roots)
        {
            if (string.IsNullOrEmpty(root))
            {
                continue;
            }

            var p = Path.GetFullPath(Path.Combine(root, "Item", "ItemValue.txt"));
            if (File.Exists(p))
            {
                return p;
            }
        }

        return null;
    }
}
