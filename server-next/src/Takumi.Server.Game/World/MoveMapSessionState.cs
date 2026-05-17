using System.Collections.Concurrent;

namespace Takumi.Server.Game.World;

/// <summary>Per-presence move-map anti-tamper seed for <c>0x8E 02</c> block key.</summary>
public static class MoveMapSessionState
{
    static readonly ConcurrentDictionary<Guid, uint> Seeds = new();
    static readonly ConcurrentDictionary<Guid, byte> TeleportInProgress = new();

    public static void Reset(Guid presenceSessionId, uint seed) => Seeds[presenceSessionId] = seed;

    public static bool TryGet(Guid presenceSessionId, out uint seed) => Seeds.TryGetValue(presenceSessionId, out seed);

    public static void Remove(Guid presenceSessionId)
    {
        Seeds.TryRemove(presenceSessionId, out _);
        TeleportInProgress.TryRemove(presenceSessionId, out _);
    }

    public static void SetTeleportInProgress(Guid presenceSessionId, bool active)
    {
        if (active)
        {
            TeleportInProgress[presenceSessionId] = 1;
        }
        else
        {
            TeleportInProgress.TryRemove(presenceSessionId, out _);
        }
    }

    public static bool IsTeleportInProgress(Guid presenceSessionId) =>
        TeleportInProgress.ContainsKey(presenceSessionId);

    public static bool SkipKeyCheck() =>
        string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_MOVE_MAP_SKIP_KEY_CHECK"),
            "1",
            StringComparison.OrdinalIgnoreCase);
}
