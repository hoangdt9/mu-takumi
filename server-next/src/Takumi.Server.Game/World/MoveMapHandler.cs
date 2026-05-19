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
        Action? onRosterSave,
        CancellationToken ct)
    {
        if (!GamePacketFinders.TryFindMoveMapRequest(packet.AsSpan(), out var moveOff, out var blockKey, out var mapIdx))
        {
            return false;
        }

        if (MoveMapSessionState.IsTeleportInProgress(presenceSessionId))
        {
            await WriteAsync(connection, clientProtectOutbound, writeAsync, MoveMapWire602.BuildAnswer(MoveMapWire602.ResultFailedTeleport), ct)
                .ConfigureAwait(false);
            Console.WriteLine(
                "[m8] move map denied index={0} concurrent teleport {1} frame@{2}",
                mapIdx,
                remote,
                moveOff);
            return true;
        }

        MoveMapSessionState.SetTeleportInProgress(presenceSessionId, true);
        try
        {
        if (!MoveMapKeyGenerator.TryValidateBlockKey(presenceSessionId, blockKey, out var usedLegacySeedZero))
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

        if (usedLegacySeedZero)
        {
            Console.WriteLine(
                "[m8] move map key accepted via legacy seed=0 (no 8E 01 or mismatch) index={0} key=0x{1:X8} {2}",
                mapIdx,
                blockKey,
                remote);
        }

        if (!string.IsNullOrEmpty(accountId))
        {
            await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
                .ConfigureAwait(false);
        }

        if (PlayerUiSession.IsWarehouseOnlyMoveBlock(presenceSessionId))
        {
            WarehouseGameplayHandler.CloseVaultIfOpen(presenceSessionId, accountId, "move-map-stale", remote);
        }

        var prevMap = player.MapId;
        var moveCtx = MoveMapSessionRules.BuildContext(player, presenceSessionId, teleportLockHeldByCaller: true);
        byte result;
        int zenCost = 0;
        MapGateService.TeleportDestination dest = default;
        if (MoveMapService.TryResolve(
                mapIdx,
                moveCtx,
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

        var wireGold = (uint)Math.Clamp(player.Zen, 0, uint.MaxValue);
        await WriteAsync(
                connection,
                clientProtectOutbound,
                writeAsync,
                ShopCommerceWire602.BuildRepair(wireGold),
                ct)
            .ConfigureAwait(false);

        player.MapId = dest.MapId;
        player.PosX = dest.X;
        player.PosY = dest.Y;
        player.Angle = dest.Angle;
        PlayerWalkHandler.HealSpawnTile(player);
        onRosterDirty?.Invoke();
        onRosterSave?.Invoke();

        await WriteAsync(connection, clientProtectOutbound, writeAsync, MoveMapWire602.BuildAnswer(result), ct)
            .ConfigureAwait(false);

        // Cross-map: C1 0x1C flag=1 then F3 03 reload. Same-map: skip 0x1C flag=0 (client mis-parses XY —
        // see IMPLEMENTATION-CHECKLIST M7d / ReceiveTeleport); F3 03 in-place reposition only.
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

        await MoveMapPostWarp.SendPersonalShopViewportRedrawAsync(
                connection,
                clientProtectOutbound,
                writeAsync,
                ct)
            .ConfigureAwait(false);

        var flag = (ushort)(dest.MapChanged ? 1 : 0);

        if (!MoveMapSessionState.SkipKeyCheck())
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
        finally
        {
            MoveMapSessionState.SetTeleportInProgress(presenceSessionId, false);
        }
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
