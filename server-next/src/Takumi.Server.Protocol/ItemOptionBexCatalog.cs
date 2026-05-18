namespace Takumi.Server.Protocol;

using System.Xml.Linq;

/// <summary>Extended item options from <c>ItemOptionBEx.xml</c>.</summary>
public static class ItemOptionBexCatalog
{
    public readonly record struct BexRow(
        int ItemSet,
        int ItemMinIndex,
        int ItemMaxIndex,
        int ItemLevelMin,
        int ItemLevelMax,
        int ItemExc,
        int OptionIndex,
        int OptionValue);

    static readonly object Gate = new();
    static bool _ready;
    static List<BexRow> _rows = [];

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

    public static IReadOnlyList<BexRow> Rows
    {
        get
        {
            EnsureInitialized();
            return _rows;
        }
    }

    public static string? ResolvePath()
    {
        var explicitPath = Environment.GetEnvironmentVariable("TAKUMI_ITEM_OPTION_BEX_PATH")?.Trim();
        if (!string.IsNullOrEmpty(explicitPath) && File.Exists(explicitPath))
        {
            return Path.GetFullPath(explicitPath);
        }

        return SetItemTypeCatalog.ResolveUnderData("ItemOptionBEx.xml");
    }

    static void Load(string path)
    {
        var doc = XDocument.Load(path);
        var root = doc.Root;
        if (root is null)
        {
            return;
        }

        var list = new List<BexRow>();
        foreach (var el in root.Elements("List"))
        {
            list.Add(new BexRow(
                AttrInt(el, "ItemSet"),
                AttrInt(el, "ItemMinIndex"),
                AttrInt(el, "ItemMaxIndex"),
                AttrInt(el, "ItemLevelMin"),
                AttrInt(el, "ItemLevelMax"),
                AttrInt(el, "ItemExc"),
                AttrInt(el, "OptionIndex"),
                AttrInt(el, "OptionValue")));
        }

        _rows = list;
    }

    static int AttrInt(XElement el, string name)
    {
        var a = el.Attribute(name);
        return a is not null && int.TryParse(a.Value, out var v) ? v : -1;
    }
}
