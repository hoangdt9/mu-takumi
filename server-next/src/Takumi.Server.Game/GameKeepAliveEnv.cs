using System.Globalization;

namespace Takumi.Server.Game;

/// <summary>Parses <c>TAKUMI_GAME_KEEPALIVE_SECONDS</c> (same as LegacyLoginHost game port).</summary>
public static class GameKeepAliveEnv
{
    public static TimeSpan ParseInterval()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_GAME_KEEPALIVE_SECONDS");
        if (string.IsNullOrWhiteSpace(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var sec))
        {
            return TimeSpan.FromSeconds(25);
        }

        if (sec <= 0)
        {
            return TimeSpan.Zero;
        }

        sec = Math.Clamp(sec, 5, 600);
        return TimeSpan.FromSeconds(sec);
    }
}
