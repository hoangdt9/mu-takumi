using System.Collections.Concurrent;

namespace Takumi.Server.Game;

/// <summary>M7d: per-session death / revive timing (not persisted in roster JSON).</summary>
public static class PlayerVitalsState
{
    static readonly ConcurrentDictionary<Guid, long> ReviveAtMs = new();

    public static bool IsDead(Guid sessionId) =>
        ReviveAtMs.TryGetValue(sessionId, out var until) && until > Environment.TickCount64;

    public static void MarkDead(Guid sessionId, TimeSpan reviveDelay)
    {
        ReviveAtMs[sessionId] = Environment.TickCount64 + (long)reviveDelay.TotalMilliseconds;
    }

    public static bool TryClearDead(Guid sessionId)
    {
        return ReviveAtMs.TryRemove(sessionId, out _);
    }

    public static bool TryGetReviveDue(Guid sessionId, out bool due)
    {
        due = false;
        if (!ReviveAtMs.TryGetValue(sessionId, out var until))
        {
            return false;
        }

        if (Environment.TickCount64 >= until)
        {
            due = true;
            return true;
        }

        return true;
    }

    public static void Unregister(Guid sessionId) => ReviveAtMs.TryRemove(sessionId, out _);

    public static TimeSpan ReviveDelayFromEnv()
    {
        var sec = 5;
        var raw = Environment.GetEnvironmentVariable("TAKUMI_PLAYER_REVIVE_SECONDS");
        if (!string.IsNullOrWhiteSpace(raw) && int.TryParse(raw, out var v))
        {
            sec = Math.Clamp(v, 1, 120);
        }

        return TimeSpan.FromSeconds(sec);
    }
}
