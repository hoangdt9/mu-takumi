using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Item pick/drop/move (<c>0x22</c>–<c>0x24</c>) — inventory bag + ground items.</summary>
public static class ItemWorldHandler
{
    public static async Task<bool> TryHandlePacketAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        byte[] packet,
        string remote,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        CancellationToken ct)
    {
        if (ClientGameplayPackets602.TryFindItemMoveRequest(
                packet,
                out _,
                out var srcFlag,
                out var srcSlot,
                out var dstFlag,
                out var dstSlot))
        {
            return await HandleMoveAsync(
                    player,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    srcFlag,
                    srcSlot,
                    dstFlag,
                    dstSlot,
                    writeAsync,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        if (ClientGameplayPackets602.TryFindItemDropRequest(packet, out _, out var dropX, out var dropY, out var dropSlot))
        {
            return await HandleDropAsync(
                    player,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    dropX,
                    dropY,
                    dropSlot,
                    writeAsync,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        if (ClientGameplayPackets602.TryFindItemPickRequest(packet, out _, out var mapItemIndex))
        {
            return await HandlePickAsync(
                    player,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    mapItemIndex,
                    writeAsync,
                    onRosterDirty,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        return false;
    }

    static async Task<bool> HandleMoveAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        byte srcFlag,
        byte srcSlot,
        byte dstFlag,
        byte dstSlot,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        string remote,
        CancellationToken ct)
    {
        if (srcFlag != 0 || dstFlag != 0)
        {
            await writeAsync(ItemWorldWire602.BuildMoveFail(dstSlot), ct).ConfigureAwait(false);
            return true;
        }

        await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
            .ConfigureAwait(false);

        if (!PlayerShopSession.TryMoveInventorySlot(presenceSessionId, srcSlot, dstSlot, out var targetItem))
        {
            await writeAsync(ItemWorldWire602.BuildMoveFail(dstSlot), ct).ConfigureAwait(false);
            Console.WriteLine("[m7] item move fail {0}→{1} {2}", srcSlot, dstSlot, remote);
            return true;
        }

        PersistSlotMirror(presenceSessionId, accountId, characterName10, srcSlot);
        PersistSlotMirror(presenceSessionId, accountId, characterName10, dstSlot);

        await writeAsync(ItemWorldWire602.BuildMoveSuccess(dstSlot, targetItem), ct).ConfigureAwait(false);
        Console.WriteLine("[m7] item move {0}→{1} map={2} {3}", srcSlot, dstSlot, player.MapId, remote);
        return true;
    }

    static async Task<bool> HandleDropAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        byte x,
        byte y,
        byte slot,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        string remote,
        CancellationToken ct)
    {
        await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
            .ConfigureAwait(false);

        if (!PlayerShopSession.TryGetSlot(presenceSessionId, slot, out var blob) || ItemWire602.IsEmpty(blob))
        {
            await writeAsync(ItemWorldWire602.BuildDropFail(slot), ct).ConfigureAwait(false);
            return true;
        }

        var itemCopy = blob.ToArray();
        var empty = new byte[ItemWire602.WireBytes];
        PlayerShopSession.SetSlot(presenceSessionId, slot, empty);
        PlayerShopSession.PersistSlotToMirror(accountId, characterName10, slot, empty);

        var dropX = x > 0 ? x : player.PosX;
        var dropY = y > 0 ? y : player.PosY;
        var mapIndex = MapGroundItemStore.Drop(player.MapId, dropX, dropY, itemCopy);
        if (mapIndex == 0)
        {
            PlayerShopSession.SetSlot(presenceSessionId, slot, itemCopy);
            PlayerShopSession.PersistSlotToMirror(accountId, characterName10, slot, itemCopy);
            await writeAsync(ItemWorldWire602.BuildDropFail(slot), ct).ConfigureAwait(false);
            return true;
        }

        await writeAsync(ItemWorldWire602.BuildDropSuccess(slot), ct).ConfigureAwait(false);
        await writeAsync(ItemViewportWire602.BuildCreateSingle(mapIndex, dropX, dropY, itemCopy), ct)
            .ConfigureAwait(false);
        Console.WriteLine("[m7] item drop slot={0} mapIdx={1} xy=({2},{3}) map={4} {5}", slot, mapIndex, dropX, dropY, player.MapId, remote);
        return true;
    }

    static async Task<bool> HandlePickAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        ushort mapItemIndex,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        string remote,
        CancellationToken ct)
    {
        if (mapItemIndex == 0)
        {
            await writeAsync(ItemWorldWire602.BuildPickFail(), ct).ConfigureAwait(false);
            return true;
        }

        if (!MapGroundItemStore.TryTake(player.MapId, mapItemIndex, player.PosX, player.PosY, out var item12))
        {
            await writeAsync(ItemWorldWire602.BuildPickFail(), ct).ConfigureAwait(false);
            Console.WriteLine("[m7] item pick fail idx={0} map={1} {2}", mapItemIndex, player.MapId, remote);
            return true;
        }

        await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
            .ConfigureAwait(false);

        if (!PlayerShopSession.TryFindEmptyBagSlot(presenceSessionId, out var bagSlot))
        {
            MapGroundItemStore.Drop(player.MapId, player.PosX, player.PosY, item12);
            await writeAsync(ItemWorldWire602.BuildPickFail(), ct).ConfigureAwait(false);
            return true;
        }

        PlayerShopSession.SetSlot(presenceSessionId, bagSlot, item12);
        PlayerShopSession.PersistSlotToMirror(accountId, characterName10, bagSlot, item12);

        await writeAsync(ItemWorldWire602.BuildPick(bagSlot, item12), ct).ConfigureAwait(false);
        await writeAsync(ItemViewportWire602.BuildDeleteSingle(mapItemIndex), ct).ConfigureAwait(false);
        Console.WriteLine("[m7] item pick idx={0} → inv={1} map={2} {3}", mapItemIndex, bagSlot, player.MapId, remote);
        return true;
    }

    static void PersistSlotMirror(Guid sessionId, string? accountId, byte[] characterName10, byte slot)
    {
        if (PlayerShopSession.TryGetSlot(sessionId, slot, out var blob) && !ItemWire602.IsEmpty(blob))
        {
            PlayerShopSession.PersistSlotToMirror(accountId, characterName10, slot, blob);
        }
        else
        {
            PlayerShopSession.PersistSlotToMirror(accountId, characterName10, slot, new byte[ItemWire602.WireBytes]);
        }
    }
}
