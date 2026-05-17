using MUnique.OpenMU.Network;
using Takumi.Server.Game.Networking;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Client walk (<c>C1 … 0xD4</c>) validation + echo (parity legacy GS move ack).</summary>
public static class PlayerWalkHandler
{
    public static async Task HandleWalkAsync(
        Guid presenceSessionId,
        Connection connection,
        (byte K1, byte K2)? protect,
        GameRosterEntry entry,
        byte srcX,
        byte srcY,
        byte endX,
        byte endY,
        byte angle1To8,
        bool changedPosition,
        string remote,
        MonsterViewportTracker monsterViewportTracker,
        CancellationToken ct)
    {
        var mapId = entry.MapId;
        var heroKey = MonsterViewerRegistry.TryGetClientHeroWireKey(presenceSessionId, out var hk)
            ? hk
            : 0;

        byte ackX = entry.PosX;
        byte ackY = entry.PosY;
        var ackAngle = angle1To8;

        if (changedPosition)
        {
            var srcOk = MapAttWalkability.CanWalk(mapId, srcX, srcY);
            var endOk = MapAttWalkability.CanWalk(mapId, endX, endY);
            var srcMatches = srcX == entry.PosX && srcY == entry.PosY;

            if (endOk && (srcOk || srcMatches))
            {
                ackX = endX;
                ackY = endY;
                entry.PosX = endX;
                entry.PosY = endY;
                await MapMonsterScopeSender.TrySendOnMoveAsync(
                        monsterViewportTracker,
                        connection,
                        protect,
                        mapId,
                        endX,
                        endY,
                        remote,
                        ct)
                    .ConfigureAwait(false);
                MonsterViewerRegistry.UpdatePosition(presenceSessionId, mapId, endX, endY);
                await GameMapPresenceRegistry.BroadcastPositionAsync(
                        presenceSessionId,
                        mapId,
                        endX,
                        endY,
                        remote,
                        ct)
                    .ConfigureAwait(false);
            }
            else if (!endOk)
            {
                Console.WriteLine(
                    "[{0}] [m4-walk] denied map={1} ({2},{3})→({4},{5}) blocked srcOk={6} srcMatch={7}",
                    remote,
                    mapId,
                    srcX,
                    srcY,
                    endX,
                    endY,
                    srcOk,
                    srcMatches);
            }
        }

        entry.Angle = ackAngle;

        var echo = MonsterWalkWire602.BuildWithFacing(heroKey, ackX, ackY, ackAngle);
        await GamePortOutboundWire.WriteAsync(connection, protect, echo, ct).ConfigureAwait(false);
    }

    /// <summary>Heal spawn/warp tile when ATT marks the roster position blocked (bridge edges, gate fallbacks).</summary>
    public static void HealSpawnTile(GameRosterEntry entry)
    {
        if (MapAttWalkability.CanWalk(entry.MapId, entry.PosX, entry.PosY))
        {
            return;
        }

        if (MapAttWalkability.TryFindNearestWalkable(entry.MapId, entry.PosX, entry.PosY, out var wx, out var wy))
        {
            Console.WriteLine(
                "[m4-walk] spawn heal map={0} ({1},{2})→({3},{4}) name='{5}'",
                entry.MapId,
                entry.PosX,
                entry.PosY,
                wx,
                wy,
                System.Text.Encoding.ASCII.GetString(entry.Name10).TrimEnd('\0'));
            entry.PosX = wx;
            entry.PosY = wy;
        }
    }
}
