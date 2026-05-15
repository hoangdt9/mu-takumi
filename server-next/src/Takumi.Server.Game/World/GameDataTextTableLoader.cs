using System.Globalization;

namespace Takumi.Server.Game.World;

/// <summary>Shared line parser for GameServer <c>MemScript</c>-style tables (<c>Gate.txt</c>, <c>ShopManager.txt</c>, …).</summary>
internal static class GameDataTextTableLoader
{
    public static IEnumerable<string[]> ReadDataRows(string path)
    {
        foreach (var line in File.ReadAllLines(path))
        {
            var trimmed = StripComment(line).Trim();
            if (trimmed.Length == 0)
            {
                continue;
            }

            if (string.Equals(trimmed, "end", StringComparison.OrdinalIgnoreCase))
            {
                yield break;
            }

            if (trimmed.StartsWith("//", StringComparison.Ordinal))
            {
                continue;
            }

            var parts = trimmed.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length > 0)
            {
                yield return parts;
            }
        }
    }

    public static int ParseInt(string s) =>
        int.Parse(s, NumberStyles.Integer, CultureInfo.InvariantCulture);

    public static int ParseIntOrStar(string s) =>
        string.Equals(s, "*", StringComparison.Ordinal) ? -1 : ParseInt(s);

    public static short? ParseShortOrStarNullable(string s)
    {
        if (string.Equals(s, "*", StringComparison.Ordinal))
        {
            return null;
        }

        return (short)ParseInt(s);
    }

    public static string StripComment(string line)
    {
        var slash = line.IndexOf("//", StringComparison.Ordinal);
        return slash >= 0 ? line[..slash] : line;
    }
}
