using System.Globalization;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Item footprint (columns × rows) — client <c>Item_*.bmd</c> first, then <c>Item/Item.txt</c> fallback.</summary>
public static class ItemSizeCatalog
{
    sealed record SizeRow(int Width, int Height);

    static readonly object Gate = new();
    static Dictionary<int, SizeRow> _byIndex = new();
    static string? _sourceLabel;

    public static void EnsureInitialized()
    {
        if (_byIndex.Count > 0)
        {
            return;
        }

        lock (Gate)
        {
            if (_byIndex.Count > 0)
            {
                return;
            }

            var (map, label) = LoadBestSource();
            _byIndex = map ?? new Dictionary<int, SizeRow>();
            _sourceLabel = label;
            Console.WriteLine("[m8] ItemSizeCatalog: {0} item footprints (source={1})", _byIndex.Count, _sourceLabel ?? "none");
        }
    }

    public static bool TryGetSize(int itemIndex, out int width, out int height)
    {
        EnsureInitialized();
        if (_byIndex.TryGetValue(itemIndex, out var row))
        {
            width = Math.Clamp(row.Width, 1, InventoryBagGrid.Columns);
            height = Math.Clamp(row.Height, 1, InventoryBagGrid.Rows);
            return true;
        }

        width = 1;
        height = 1;
        return false;
    }

    public static void GetSize(ReadOnlySpan<byte> item12, out int width, out int height)
    {
        var index = ItemWire602.DecodeItemIndex(item12);
        if (index < 0 || !TryGetSize(index, out width, out height))
        {
            width = 1;
            height = 1;
        }
    }

    static (Dictionary<int, SizeRow>? Map, string? Label) LoadBestSource()
    {
        var bmdPath = ClientItemFootprintCatalog.ResolveBmdPath();
        Dictionary<int, (int Width, int Height)>? bmd = null;
        if (bmdPath is not null)
        {
            bmd = ClientItemFootprintCatalog.TryLoadFromBmd(bmdPath);
            if (bmd is { Count: > 0 })
            {
                var txtPath = ResolveItemTxtPath();
                if (txtPath is not null)
                {
                    var txt = LoadFromItemTxt(txtPath);
                    var mismatches = CountMismatches(bmd, txt);
                    if (mismatches > 0)
                    {
                        Console.WriteLine(
                            "[m8] ItemSizeCatalog: client BMD vs Item.txt — {0} footprint mismatch(es); using BMD ({1})",
                            mismatches,
                            bmdPath);
                    }
                }

                return (ToSizeRows(bmd), $"client-bmd:{Path.GetFileName(bmdPath)}");
            }

            Console.WriteLine("[m8] ItemSizeCatalog: failed to read client BMD at {0}", bmdPath);
        }

        var itemTxt = ResolveItemTxtPath();
        if (itemTxt is not null)
        {
            var fromTxt = LoadFromItemTxt(itemTxt);
            if (fromTxt.Count > 0)
            {
                return (fromTxt, $"item-txt:{Path.GetFileName(itemTxt)}");
            }
        }

        return (null, "default-1x1");
    }

    static Dictionary<int, SizeRow> ToSizeRows(Dictionary<int, (int Width, int Height)> src)
    {
        var map = new Dictionary<int, SizeRow>(src.Count);
        foreach (var kv in src)
        {
            map[kv.Key] = new SizeRow(kv.Value.Width, kv.Value.Height);
        }

        return map;
    }

    static int CountMismatches(
        Dictionary<int, (int Width, int Height)> bmd,
        Dictionary<int, SizeRow> txt)
    {
        var n = 0;
        foreach (var kv in txt)
        {
            if (!bmd.TryGetValue(kv.Key, out var b))
            {
                continue;
            }

            if (b.Width != kv.Value.Width || b.Height != kv.Value.Height)
            {
                n++;
            }
        }

        return n;
    }

    static Dictionary<int, SizeRow> LoadFromItemTxt(string path)
    {
        var map = new Dictionary<int, SizeRow>();
        var group = -1;
        foreach (var rawLine in File.ReadLines(path))
        {
            var line = rawLine.Trim();
            if (line.Length == 0 || line.StartsWith("//", StringComparison.Ordinal))
            {
                continue;
            }

            if (string.Equals(line, "end", StringComparison.OrdinalIgnoreCase))
            {
                group = -1;
                continue;
            }

            if (int.TryParse(line, NumberStyles.Integer, CultureInfo.InvariantCulture, out var section)
                && !line.Contains('\t')
                && section >= 0
                && section <= 20)
            {
                group = section;
                continue;
            }

            if (group < 0)
            {
                continue;
            }

            var parts = line.Split('\t');
            if (parts.Length < 5)
            {
                continue;
            }

            if (!int.TryParse(parts[0], NumberStyles.Integer, CultureInfo.InvariantCulture, out var indexInGroup))
            {
                continue;
            }

            if (!int.TryParse(parts[3], NumberStyles.Integer, CultureInfo.InvariantCulture, out var w)
                || !int.TryParse(parts[4], NumberStyles.Integer, CultureInfo.InvariantCulture, out var h))
            {
                continue;
            }

            var fullIndex = (group * 512) + indexInGroup;
            map[fullIndex] = new SizeRow(Math.Max(1, w), Math.Max(1, h));
        }

        return map;
    }

    static string? ResolveItemTxtPath()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_ITEM_TXT_PATH")?.Trim();
        if (!string.IsNullOrEmpty(env) && File.Exists(env))
        {
            return env;
        }

        var roots = new[]
        {
            Environment.GetEnvironmentVariable("TAKUMI_GAMESERVER_DATA_PATH")?.Trim(),
            "/muserver-data",
            Path.Combine(Environment.CurrentDirectory, "Data"),
            Path.Combine(Environment.CurrentDirectory, "..", "MuServer", "4.GameServer", "Data"),
            Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "..", "..", "MuServer", "4.GameServer", "Data"),
        };

        foreach (var root in roots)
        {
            if (string.IsNullOrEmpty(root))
            {
                continue;
            }

            var p = Path.GetFullPath(Path.Combine(root, "Item", "Item.txt"));
            if (File.Exists(p))
            {
                return p;
            }
        }

        return null;
    }
}
