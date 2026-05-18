namespace Takumi.Server.Protocol;

/// <summary>Jewel of Harmony option tables (<c>JewelOfHarmonyOption.txt</c>).</summary>
public static class JewelOfHarmonyOptionCatalog
{
    public readonly record struct HarmonyOptionDef(int Index, int[] ValueTable);

    static readonly object Gate = new();
    static bool _ready;
    static HarmonyOptionDef?[][] _types = Array.Empty<HarmonyOptionDef?[]>();

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

    public static HarmonyOptionDef? Get(int type, int index)
    {
        EnsureInitialized();
        if (type < 0 || type >= _types.Length)
        {
            return null;
        }

        if (index < 0 || index >= _types[type].Length)
        {
            return null;
        }

        return _types[type][index];
    }

    public static string? ResolvePath()
    {
        var explicitPath = Environment.GetEnvironmentVariable("TAKUMI_JEWEL_OF_HARMONY_OPTION_PATH")?.Trim();
        if (!string.IsNullOrEmpty(explicitPath) && File.Exists(explicitPath))
        {
            return Path.GetFullPath(explicitPath);
        }

        return SetItemTypeCatalog.ResolveUnderData("JewelOfHarmonyOption.txt");
    }

    static void Load(string path)
    {
        var text = File.ReadAllText(path);
        var blocks = text.Split("end", StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
        var types = new List<HarmonyOptionDef?[]>();

        foreach (var block in blocks)
        {
            var lines = block.Split('\n', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
            if (lines.Length == 0)
            {
                continue;
            }

            if (!int.TryParse(lines[0], out var section) || section <= 0)
            {
                continue;
            }

            var current = new List<HarmonyOptionDef?>();
            for (var li = 1; li < lines.Length; li++)
            {
                var line = lines[li].Trim();
                if (line.Length == 0 || line.StartsWith("//", StringComparison.Ordinal))
                {
                    continue;
                }

                var parts = SplitTokens(line);
                if (parts.Count < 4 || !int.TryParse(parts[0], out var index))
                {
                    continue;
                }

                var values = new int[16];
                var vi = 0;
                for (var pi = 3; pi < parts.Count && vi < values.Length; pi++)
                {
                    if (int.TryParse(parts[pi], out var v))
                    {
                        values[vi++] = v;
                    }
                    else if (pi + 1 < parts.Count && int.TryParse(parts[pi + 1], out _))
                    {
                        pi++;
                        if (int.TryParse(parts[pi], out var v2))
                        {
                            values[vi++] = v2;
                        }
                    }
                }

                while (current.Count <= index)
                {
                    current.Add(null);
                }

                current[index] = new HarmonyOptionDef(index, values);
            }

            while (types.Count < section)
            {
                types.Add(Array.Empty<HarmonyOptionDef?>());
            }

            types[section - 1] = BuildType(current);
        }

        _types = types.ToArray();
    }

    static HarmonyOptionDef?[] BuildType(List<HarmonyOptionDef?> list)
    {
        if (list.Count == 0)
        {
            return Array.Empty<HarmonyOptionDef?>();
        }

        var max = list.Count - 1;
        var arr = new HarmonyOptionDef?[max + 1];
        for (var i = 0; i <= max; i++)
        {
            arr[i] = i < list.Count ? list[i] : null;
        }

        return arr;
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
}
