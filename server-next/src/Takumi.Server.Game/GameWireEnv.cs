using System.Globalization;
using System.Text;

namespace Takumi.Server.Game;

/// <summary>Shared wire env parsing for game/login hosts (join version bytes, serial).</summary>
public static class GameWireEnv
{
    public static byte[]? ParseJoinVersion5(string? s) => ParseHexOrAscii5(s);

    public static byte[]? ParseSerial16(string? s)
    {
        if (string.IsNullOrWhiteSpace(s))
        {
            return null;
        }

        var t = s.Trim();
        if (t.Length == 16 && t.All(static c => c < 128))
        {
            return Encoding.ASCII.GetBytes(t);
        }

        return null;
    }

    static byte[]? ParseHexOrAscii5(string? s)
    {
        if (string.IsNullOrWhiteSpace(s))
        {
            return null;
        }

        s = s.Trim();
        if (s.Length == 5 && s.All(c => c < 128))
        {
            return Encoding.ASCII.GetBytes(s);
        }

        var parts = s.Split(new[] { ' ', ',', ';' }, StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
        if (parts.Length == 5 && parts.All(p => byte.TryParse(p, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out _)))
        {
            return parts.Select(p => byte.Parse(p, NumberStyles.HexNumber, CultureInfo.InvariantCulture)).ToArray();
        }

        if (parts.Length == 5)
        {
            try
            {
                return Convert.FromHexString(string.Concat(parts));
            }
            catch
            {
                return null;
            }
        }

        try
        {
            var bytes = Convert.FromHexString(s.Replace(" ", string.Empty, StringComparison.Ordinal));
            return bytes.Length == 5 ? bytes : null;
        }
        catch
        {
            return null;
        }
    }

    public static Dictionary<string, string>? ParseAccounts(string? s)
    {
        if (string.IsNullOrWhiteSpace(s))
        {
            return null;
        }

        var d = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
        foreach (var part in s.Split(new[] { ';', '|' }, StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            var idx = part.IndexOf(':');
            if (idx <= 0)
            {
                continue;
            }

            d[part[..idx].Trim()] = part[(idx + 1)..].Trim();
        }

        return d.Count > 0 ? d : null;
    }
}
