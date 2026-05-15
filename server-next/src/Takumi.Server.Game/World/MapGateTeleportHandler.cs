using MUnique.OpenMU.Network;
using Takumi.Server.Game.Networking;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>M8: handles client gate teleport (<c>C1/C3 0x1C</c>, gate &gt; 0).</summary>
public static class MapGateTeleportHandler
{
    public static async Task<bool> TryHandleAsync(
        byte[] packet,
        GameRosterEntry character,
        MonsterViewportTracker monsterTracker,
        Guid presenceSessionId,
        Connection connection,
        (byte K1, byte K2)? protect,
        string remote,
        bool verbose,
        CancellationToken ct)
    {
        if (!GamePacketFinders.TryFindTeleportGateRequest(packet, out var gateIndex, out _, out _))
        {
            return false;
        }

        if (gateIndex == 0)
        {
            return false;
        }

        var result = MapGateTeleportService.TryTeleport(
            gateIndex,
            character.MapId,
            character.PosX,
            character.PosY,
            character.Angle,
            character.Level);

        var pkt = TeleportWire602.Build(result.ClientFlag, result.MapId, result.X, result.Y, result.Dir);
        await GamePortOutboundWire.WriteAsync(connection, protect, pkt, ct).ConfigureAwait(false);

        if (result.Accepted)
        {
            var mapChanged = result.MapId != character.MapId;
            character.MapId = result.MapId;
            character.PosX = result.X;
            character.PosY = result.Y;
            character.Angle = result.Dir;
            GameMapPresenceRegistry.UpdateMap(presenceSessionId, result.MapId, result.X, result.Y, result.Dir);
            if (mapChanged)
            {
                await MapMonsterScopeSender.TrySendAfterJoinAsync(
                        monsterTracker,
                        connection,
                        protect,
                        result.MapId,
                        result.X,
                        result.Y,
                        remote,
                        ct)
                    .ConfigureAwait(false);
            }
            else
            {
                await MapMonsterScopeSender.TrySendOnMoveAsync(
                        monsterTracker,
                        connection,
                        protect,
                        result.MapId,
                        result.X,
                        result.Y,
                        remote,
                        ct)
                    .ConfigureAwait(false);
            }
        }

        if (verbose)
        {
            Console.WriteLine(
                "[{0}] [m8] gate teleport gate={1} ok={2} map={3} xy=({4},{5}) flag={6}",
                remote,
                gateIndex,
                result.Accepted,
                result.MapId,
                result.X,
                result.Y,
                result.ClientFlag);
        }

        return true;
    }
}
