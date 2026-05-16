using MUnique.OpenMU.Network;
using Takumi.Server.Game.Networking;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Move-map warp (<c>C1 0A 8E 02</c> → <c>8E 03</c> + teleport / join-map).</summary>
public static class MoveMapHandler
{
    public static async Task<bool> TryHandleAsync(
        GameRosterEntry player,
        MonsterViewportTracker tracker,
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        string? accountId,
        byte[] characterName10,
        Guid presenceSessionId,
        byte[] packet,
        string remote,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        CancellationToken ct)
    {
        if (!GamePacketFinders.TryFindMoveMapRequest(packet.AsSpan(), out var moveOff, out var blockKey, out var mapIdx))
        {
            return false;
        }

        if (!MoveMapSessionState.SkipKeyCheck())
        {
            if (!MoveMapSessionState.TryGet(presenceSessionId, out var seed)
                || !MoveMapKeyGenerator.TryAcceptKey(ref seed, blockKey))
            {
                await WriteAsync(connection, clientProtectOutbound, writeAsync, MoveMapWire602.BuildAnswer(MoveMapWire602.ResultFailed), ct)
                    .ConfigureAwait(false);
                Console.WriteLine(
                    "[m8] move map denied index={0} bad block key=0x{1:X8} {2} frame@{3}",
                    mapIdx,
                    blockKey,
                    remote,
                    moveOff);
                return true;
            }

            MoveMapSessionState.Reset(presenceSessionId, seed);
        }

        var prevMap = player.MapId;
        byte result;
        int zenCost = 0;
        MapGateService.TeleportDestination dest = default;
        if (MoveMapService.TryResolve(
                mapIdx,
                player.Level,
                player.Zen,
                prevMap,
                out dest,
                out var deny,
                out zenCost))
        {
            result = MoveMapWire602.ResultSuccess;
        }
        else
        {
            result = MoveMapService.ToWireResult(deny);
            await WriteAsync(connection, clientProtectOutbound, writeAsync, MoveMapWire602.BuildAnswer(result), ct)
                .ConfigureAwait(false);
            Console.WriteLine(
                "[m8] move map denied index={0} reason={1} result=0x{2:X2} {3} frame@{4}",
                mapIdx,
                deny,
                result,
                remote,
                moveOff);
            return true;
        }

        if (zenCost > 0)
        {
            player.Zen = Math.Max(0, player.Zen - zenCost);
        }

        player.MapId = dest.MapId;
        player.PosX = dest.X;
        player.PosY = dest.Y;
        player.Angle = dest.Angle;
        onRosterDirty?.Invoke();

        await WriteAsync(connection, clientProtectOutbound, writeAsync, MoveMapWire602.BuildAnswer(result), ct)
            .ConfigureAwait(false);

        var flag = (ushort)(dest.MapChanged ? 1 : 0);
        var tele = TeleportWire602.Build(flag, dest.MapId, dest.X, dest.Y, dest.Angle);
        await WriteAsync(connection, clientProtectOutbound, writeAsync, tele, ct).ConfigureAwait(false);

        if (dest.MapChanged && accountId is not null)
        {
            tracker.ResetForMap(dest.MapId, dest.X, dest.Y);
            var spawn = new JoinMapSpawnWire(dest.MapId, dest.X, dest.Y, dest.Angle);
            var joinPkt = JoinMapServerWire602.Build(player.ToWireWithSheet(), spawn);
            var invPkt = await JoinInventoryPacket602
                .BuildAsync(TakumiPostgresMirror.InventorySlots, accountId, characterName10, ct)
                .ConfigureAwait(false);
            await WriteAsync(connection, clientProtectOutbound, writeAsync, joinPkt, ct).ConfigureAwait(false);
            await WriteAsync(connection, clientProtectOutbound, writeAsync, invPkt, ct).ConfigureAwait(false);
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

        Console.WriteLine(
            "[m8] move map index={0} gate→map={1} xy=({2},{3}) zen={4} flag={5} {6} frame@{7}",
            mapIdx,
            dest.MapId,
            dest.X,
            dest.Y,
            zenCost,
            flag,
            remote,
            moveOff);
        return true;
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
