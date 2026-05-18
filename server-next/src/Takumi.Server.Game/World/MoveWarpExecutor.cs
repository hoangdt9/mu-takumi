using MUnique.OpenMU.Network;
using Takumi.Server.Game.Networking;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Shared post-warp client sync (move-map UI + custom NPC move).</summary>
public static class MoveWarpExecutor
{
    public static async Task ApplyAsync(
        GameRosterEntry player,
        MonsterViewportTracker tracker,
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        string? accountId,
        byte[] characterName10,
        Guid presenceSessionId,
        MapGateService.TeleportDestination dest,
        byte previousMap,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        string remote,
        Action? onRosterDirty,
        Action? onRosterSave,
        bool reseedMoveMapKey,
        string logPrefix,
        CancellationToken ct)
    {
        dest = dest with { MapChanged = dest.MapId != previousMap };

        player.MapId = dest.MapId;
        player.PosX = dest.X;
        player.PosY = dest.Y;
        player.Angle = dest.Angle;
        PlayerWalkHandler.HealSpawnTile(player);
        onRosterDirty?.Invoke();
        onRosterSave?.Invoke();

        if (dest.MapChanged)
        {
            var tele = TeleportWire602.Build(1, dest.MapId, dest.X, dest.Y, dest.Angle);
            await WriteAsync(connection, clientProtectOutbound, writeAsync, tele, ct).ConfigureAwait(false);
        }

        if (accountId is not null)
        {
            await MoveWarpJoinReload.SendAsync(
                    player,
                    tracker,
                    connection,
                    clientProtectOutbound,
                    accountId,
                    characterName10,
                    presenceSessionId,
                    dest,
                    writeAsync,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }
        else if (!dest.MapChanged)
        {
            var joinPkt = JoinMapServerWire602.Build(
                player.ToWireWithSheet(),
                new JoinMapSpawnWire(dest.MapId, dest.X, dest.Y, dest.Angle));
            await WriteAsync(connection, clientProtectOutbound, writeAsync, joinPkt, ct).ConfigureAwait(false);
        }

        await MoveMapPostWarp.SendPersonalShopViewportRedrawAsync(
                connection,
                clientProtectOutbound,
                writeAsync,
                ct)
            .ConfigureAwait(false);

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

        if (reseedMoveMapKey && !MoveMapSessionState.SkipKeyCheck())
        {
            await MoveMapOutbound.TrySendChecksumAfterJoinAsync(
                    presenceSessionId,
                    connection,
                    clientProtectOutbound,
                    writeAsync: null,
                    ct)
                .ConfigureAwait(false);
        }

        Console.WriteLine(
            "[m8] {0} map={1} xy=({2},{3}) flag={4} {5}",
            logPrefix,
            dest.MapId,
            dest.X,
            dest.Y,
            dest.MapChanged ? 1 : 0,
            remote);
    }

    static Task WriteAsync(
        Connection? connection,
        (byte K1, byte K2)? protect,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        byte[] packet,
        CancellationToken ct) =>
        connection is not null
            ? GamePortOutboundWire.WriteAsync(connection, protect, packet, ct)
            : writeAsync(packet, ct);
}
