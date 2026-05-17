using MUnique.OpenMU.Network;
using Takumi.Server.Game.Networking;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Post-warp client resync: <c>F3 03</c>/<c>F3 10</c> + monster scope (avoids broken <c>0x1C flag=0</c> on same map).</summary>
internal static class MoveWarpJoinReload
{
    public static async Task SendAsync(
        GameRosterEntry player,
        MonsterViewportTracker tracker,
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        string accountId,
        byte[] characterName10,
        Guid presenceSessionId,
        MapGateService.TeleportDestination dest,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        string remote,
        CancellationToken ct)
    {
        tracker.ResetForMap(dest.MapId, dest.X, dest.Y);
        var spawn = new JoinMapSpawnWire(dest.MapId, dest.X, dest.Y, dest.Angle);
        var joinPkt = JoinMapServerWire602.Build(player.ToWireWithSheet(), spawn);
        var invPkt = await JoinInventoryPacket602
            .BuildAsync(TakumiPostgresMirror.InventorySlots, accountId, characterName10, ct)
            .ConfigureAwait(false);

        if (connection is not null)
        {
            await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, joinPkt, ct).ConfigureAwait(false);
            await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, invPkt, ct).ConfigureAwait(false);
        }
        else
        {
            await writeAsync(joinPkt, ct).ConfigureAwait(false);
            await writeAsync(invPkt, ct).ConfigureAwait(false);
        }

        MonsterViewerRegistry.UpdatePosition(presenceSessionId, dest.MapId, dest.X, dest.Y);
        if (connection is not null)
        {
            await MapMonsterScopeSender.TrySendAfterJoinAsync(
                    tracker,
                    connection,
                    clientProtectOutbound,
                    dest.MapId,
                    dest.X,
                    dest.Y,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }
    }
}
