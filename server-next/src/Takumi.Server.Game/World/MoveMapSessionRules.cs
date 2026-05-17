using Takumi.Server.Game.Networking;

namespace Takumi.Server.Game.World;

/// <summary>Builds <see cref="MoveMapPlayerContext"/> and session block checks for move-map UI.</summary>
public static class MoveMapSessionRules
{
    public static bool PkLimitFreeEnabled() =>
        string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_PK_LIMIT_FREE"),
            "1",
            StringComparison.OrdinalIgnoreCase);

    public static MoveMapPlayerContext BuildContext(GameRosterEntry player, Guid presenceSessionId)
    {
        var pk = (byte)0;
        if (GameMapPresenceRegistry.TryGetSession(presenceSessionId, out var presence) && presence is not null)
        {
            pk = presence.Appearance.PkLevel;
        }

        return new MoveMapPlayerContext(
            player.Level,
            player.Zen,
            player.Reset,
            player.AccountLevel,
            pk,
            player.GensFamily,
            player.MapId,
            player.ServerClass,
            presenceSessionId,
            PlayerUiSession.IsMoveBlocked(presenceSessionId),
            PlayerVitalsState.IsDead(presenceSessionId),
            MoveMapSessionState.IsTeleportInProgress(presenceSessionId));
    }
}
