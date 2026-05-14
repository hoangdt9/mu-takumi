using System.Globalization;

namespace Takumi.Server.Game;

/// <summary>Throttled JSON (+ Postgres mirror) roster flush between disconnects (M4c).</summary>
public static class RosterPeriodicFlush
{
    /// <summary>Parse <c>TAKUMI_ROSTER_PERIODIC_SAVE_SECONDS</c>; returns <see langword="null"/> when unset or zero.</summary>
    public static TimeSpan? TryParseIntervalFromEnv()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_ROSTER_PERIODIC_SAVE_SECONDS")?.Trim();
        if (string.IsNullOrEmpty(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var sec)
            || sec <= 0)
        {
            return null;
        }

        sec = Math.Clamp(sec, 5, 3600);
        return TimeSpan.FromSeconds(sec);
    }
}
