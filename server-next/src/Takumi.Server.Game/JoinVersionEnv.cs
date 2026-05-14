using System.Globalization;
using System.Text;

namespace Takumi.Server.Game;

/// <summary>Resolves 5-byte join version from env (same semantics as LegacyLoginHost).</summary>
public static class JoinVersionEnv
{
    /// <summary>Wire bytes for ServerVersion = "1.04.05" using GameServerInfo indices → ASCII "10405".</summary>
    public static byte[] DefaultJoinVersion10405 { get; } = Encoding.ASCII.GetBytes("10405");

    public static byte[] ResolveFromEnvironment()
    {
        var parsed = ParseHexOrAscii5(Environment.GetEnvironmentVariable("TAKUMI_JOIN_VERSION_HEX"))
                     ?? ParseHexOrAscii5(Environment.GetEnvironmentVariable("TAKUMI_JOIN_VERSION"));
        if (parsed is { Length: 5 })
        {
            return parsed;
        }

        return (byte[])DefaultJoinVersion10405.Clone();
    }

    public static byte[]? ParseHexOrAscii5(string? s)
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
            var arr = new byte[5];
            for (var i = 0; i < 5; i++)
            {
                arr[i] = byte.Parse(parts[i], NumberStyles.HexNumber, CultureInfo.InvariantCulture);
            }

            return arr;
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
}
