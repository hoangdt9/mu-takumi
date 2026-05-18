namespace Takumi.Server.Protocol;

/// <summary>Set bonus tables from <c>SetItemOption.txt</c>.</summary>
public static class SetItemOptionCatalog
{
    public readonly record struct OptionEntry(int Index, int Value);

    public readonly record struct SetOptionDef(
        int Index,
        OptionEntry[][] Tables,
        OptionEntry[] FullOptions);

    const int TableCount = 6;
    const int OptionsPerTable = 2;
    const int FullCount = 5;

    static readonly object Gate = new();
    static bool _ready;
    static Dictionary<int, SetOptionDef> _sets = new();

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

    public static bool TryGet(int setIndex, out SetOptionDef def)
    {
        EnsureInitialized();
        return _sets.TryGetValue(setIndex, out def);
    }

    public static string? ResolvePath()
    {
        var explicitPath = Environment.GetEnvironmentVariable("TAKUMI_SET_ITEM_OPTION_PATH")?.Trim();
        if (!string.IsNullOrEmpty(explicitPath) && File.Exists(explicitPath))
        {
            return Path.GetFullPath(explicitPath);
        }

        return SetItemTypeCatalog.ResolveUnderData("SetItemOption.txt");
    }

    static void Load(string path)
    {
        var map = new Dictionary<int, SetOptionDef>();
        foreach (var line in File.ReadLines(path))
        {
            var t = line.Trim();
            if (t.Length == 0 || t.StartsWith("//", StringComparison.Ordinal))
            {
                continue;
            }

            var parts = SplitTokens(t);
            if (parts.Count < 3)
            {
                continue;
            }

            if (!int.TryParse(parts[0], out var index))
            {
                continue;
            }

            var cursor = 2;
            var tables = new OptionEntry[TableCount][];
            for (var ti = 0; ti < TableCount; ti++)
            {
                tables[ti] = new OptionEntry[OptionsPerTable];
                for (var oi = 0; oi < OptionsPerTable; oi++)
                {
                    if (cursor + 1 >= parts.Count)
                    {
                        break;
                    }

                    tables[ti][oi] = new OptionEntry(
                        ParseInt(parts[cursor++]),
                        ParseInt(parts[cursor++]));
                }
            }

            var full = new OptionEntry[FullCount];
            for (var fi = 0; fi < FullCount; fi++)
            {
                if (cursor + 1 >= parts.Count)
                {
                    full[fi] = new OptionEntry(-1, 0);
                    continue;
                }

                full[fi] = new OptionEntry(ParseInt(parts[cursor++]), ParseInt(parts[cursor++]));
            }

            map[index] = new SetOptionDef(index, tables, full);
        }

        _sets = map;
    }

    static List<string> SplitTokens(string line)
    {
        var list = new List<string>();
        var i = 0;
        while (i < line.Length)
        {
            while (i < line.Length && char.IsWhiteSpace(line[i]))
            {
                i++;
            }

            if (i >= line.Length)
            {
                break;
            }

            if (line[i] == '"')
            {
                i++;
                var start = i;
                while (i < line.Length && line[i] != '"')
                {
                    i++;
                }

                list.Add(line[start..i]);
                if (i < line.Length)
                {
                    i++;
                }

                continue;
            }

            var startPlain = i;
            while (i < line.Length && !char.IsWhiteSpace(line[i]))
            {
                i++;
            }

            list.Add(line[startPlain..i]);
        }

        return list;
    }

    static int ParseInt(string s) =>
        s == "*" ? -1 : int.TryParse(s, out var v) ? v : -1;
}
