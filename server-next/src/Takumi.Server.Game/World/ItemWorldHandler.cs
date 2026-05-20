using System.Text;
using Takumi.Server.Game;
using Takumi.Server.Game.Networking;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Item pick/drop/move (<c>0x22</c>–<c>0x24</c>) — inventory bag + ground items.</summary>
public static class ItemWorldHandler
{
    static bool CanUseItems(GameRosterEntry player, Guid presenceSessionId) =>
        !PlayerVitalsState.IsDead(presenceSessionId) && (player.MaxHp <= 0 || player.CurrentHp > 0);

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

        if (ClientGameplayPackets602.TryFindItemUseRequest(packet, out _, out var useSlot, out var useTarget))
        {
            return await HandleUseAsync(
                    player,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    useSlot,
                    useTarget,
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
        if (!CanUseItems(player, presenceSessionId))
        {
            await writeAsync(ItemWorldWire602.BuildMoveFail(dstSlot), ct).ConfigureAwait(false);
            return true;
        }

        var warehouseOpen = PlayerWarehouseSession.IsOpen(presenceSessionId);
        var tradeOpen = PlayerTradeSession.IsOpen(presenceSessionId);
        if (!ClientGameplayPackets602.IsSupportedItemStorage(srcFlag, warehouseOpen, tradeOpen)
            || !ClientGameplayPackets602.IsSupportedItemStorage(dstFlag, warehouseOpen, tradeOpen))
        {
            await writeAsync(ItemWorldWire602.BuildMoveFail(dstSlot), ct).ConfigureAwait(false);
            return true;
        }

        var moveResult = await TryApplyItemMoveAsync(
                presenceSessionId,
                accountId,
                characterName10,
                srcFlag,
                srcSlot,
                dstFlag,
                dstSlot,
                ct).ConfigureAwait(false);
        if (!moveResult.Ok)
        {
            await writeAsync(ItemWorldWire602.BuildMoveFail(dstSlot), ct).ConfigureAwait(false);
            Console.WriteLine(
                "[m7] item move fail f{0}:{1}→f{2}:{3}{4} {5}",
                srcFlag,
                srcSlot,
                dstFlag,
                dstSlot,
                string.IsNullOrEmpty(moveResult.FailReason) ? string.Empty : $" reason={moveResult.FailReason}",
                remote);
            return true;
        }

        if (srcFlag == ItemStorageFlags602.Inventory && dstFlag == ItemStorageFlags602.Inventory)
        {
            // Android client cannot SM-decrypt plain C4 F3 10 from game-host (encryptionPipe off).
            // 0x24 clears picked item via ReceiveEquipmentItem; F3 10 resyncs footprint grid.
            await writeAsync(
                    ItemWorldWire602.BuildMoveSuccess(dstFlag, dstSlot, moveResult.TargetItem),
                    ct)
                .ConfigureAwait(false);
            // Do not CompactBagForPlacement here — RepackBagSlots top-left repacks the item back
            // (e.g. 12→44 becomes 12 again) while 0x24 still says dstSlot=44 → client/server desync.
            var invSync = PlayerShopSession.BuildInventoryListPacket(presenceSessionId);
            await writeAsync(invSync, ct).ConfigureAwait(false);
            if (PlayerShopSession.TryGetSessionSlots(presenceSessionId, out var snap))
            {
                var bagAnchors = snap.Keys
                    .Where(ItemWire602.IsBagSlot)
                    .OrderBy(static x => x)
                    .Select(static x => x.ToString())
                    .ToArray();
                Console.WriteLine(
                    "[m7] item move inv→inv 0x24 + F3 10 len={0}B anchors=[{1}] {2}→{3} {4}",
                    invSync.Length,
                    string.Join(',', bagAnchors),
                    srcSlot,
                    dstSlot,
                    remote);
            }
            else
            {
                Console.WriteLine(
                    "[m7] item move inv→inv 0x24 + F3 10 len={0}B {1}→{2} {3}",
                    invSync.Length,
                    srcSlot,
                    dstSlot,
                    remote);
            }

            if (ItemWire602.IsWearSlot(srcSlot) || ItemWire602.IsWearSlot(dstSlot))
            {
                await CharacterCalcBroadcast602.SendAsync(player, presenceSessionId, writeAsync, ct)
                    .ConfigureAwait(false);
            }
        }
        else if (srcFlag == ItemStorageFlags602.Inventory && dstFlag == ItemStorageFlags602.Warehouse)
        {
            await writeAsync(ItemWorldWire602.BuildItemDelete(srcSlot), ct).ConfigureAwait(false);
            await writeAsync(
                    ItemWorldWire602.BuildMoveSuccess(dstFlag, dstSlot, moveResult.TargetItem),
                    ct)
                .ConfigureAwait(false);
            Console.WriteLine(
                "[m7] item move inv→wh 0x24 f2:{0} + 0x28 inv:{1} {2}",
                dstSlot,
                srcSlot,
                remote);
        }
        else if (srcFlag == ItemStorageFlags602.Warehouse && dstFlag == ItemStorageFlags602.Inventory)
        {
            await writeAsync(
                    ItemWorldWire602.BuildMoveSuccess(dstFlag, dstSlot, moveResult.TargetItem),
                    ct)
                .ConfigureAwait(false);
            var emptyWh = new byte[ItemWire602.WireBytes];
            Array.Fill(emptyWh, (byte)0xFF);
            await writeAsync(
                    ItemWorldWire602.BuildMoveSuccess(ItemStorageFlags602.Warehouse, srcSlot, emptyWh),
                    ct)
                .ConfigureAwait(false);
            Console.WriteLine(
                "[m7] item move wh→inv 0x24 f0:{0} + clear wh:{1} {2}",
                dstSlot,
                srcSlot,
                remote);
        }
        else if (srcFlag == ItemStorageFlags602.Warehouse && dstFlag == ItemStorageFlags602.Warehouse)
        {
            await writeAsync(
                    ItemWorldWire602.BuildMoveSuccess(dstFlag, dstSlot, moveResult.TargetItem),
                    ct)
                .ConfigureAwait(false);
            if (srcSlot != dstSlot)
            {
                var emptyWh = new byte[ItemWire602.WireBytes];
                Array.Fill(emptyWh, (byte)0xFF);
                await writeAsync(
                        ItemWorldWire602.BuildMoveSuccess(ItemStorageFlags602.Warehouse, srcSlot, emptyWh),
                        ct)
                    .ConfigureAwait(false);
            }

            Console.WriteLine(
                "[m7] item move wh→wh 0x24 f2:{0} + clear f2:{1} {2}",
                dstSlot,
                srcSlot,
                remote);
        }
        else
        {
            await writeAsync(ItemWorldWire602.BuildMoveSuccess(dstFlag, dstSlot, moveResult.TargetItem), ct)
                .ConfigureAwait(false);
        }

        Console.WriteLine(
            "[m7] item move f{0}:{1}→f{2}:{3} map={4} {5}",
            srcFlag,
            srcSlot,
            dstFlag,
            dstSlot,
            player.MapId,
            remote);
        return true;
    }

    static async Task<(bool Ok, byte[] TargetItem, byte[]? SwappedIntoSource, string? FailReason)> TryApplyItemMoveAsync(
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        byte srcFlag,
        byte srcSlot,
        byte dstFlag,
        byte dstSlot,
        CancellationToken ct)
    {
        if (srcFlag == ItemStorageFlags602.Inventory && dstFlag == ItemStorageFlags602.Inventory)
        {
            await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
                .ConfigureAwait(false);
            if (!PlayerShopSession.TryMoveInventorySlot(
                    presenceSessionId,
                    srcSlot,
                    dstSlot,
                    out var targetItem,
                    out var swappedIntoSource,
                    out var moveReason))
            {
                return (false, Array.Empty<byte>(), null, moveReason);
            }

            await PersistInventorySnapshotAsync(presenceSessionId, accountId, characterName10, ct).ConfigureAwait(false);
            return (true, targetItem, swappedIntoSource, null);
        }

        if (srcFlag == ItemStorageFlags602.Warehouse && dstFlag == ItemStorageFlags602.Warehouse)
        {
            await PlayerWarehouseSession.EnsureLoadedAsync(presenceSessionId, accountId, ct).ConfigureAwait(false);
            if (!PlayerWarehouseSession.TryMoveSlot(presenceSessionId, srcSlot, dstSlot, out var targetItem))
            {
                return (false, Array.Empty<byte>(), null, "warehouse-move");
            }

            PlayerWarehouseSession.PersistSlot(accountId, srcSlot, Array.Empty<byte>());
            PlayerWarehouseSession.PersistSlot(accountId, dstSlot, targetItem);
            return (true, targetItem, null, null);
        }

        if (srcFlag == ItemStorageFlags602.Inventory && dstFlag == ItemStorageFlags602.Warehouse)
        {
            await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
                .ConfigureAwait(false);
            await PlayerWarehouseSession.EnsureLoadedAsync(presenceSessionId, accountId, ct).ConfigureAwait(false);
            if (!PlayerShopSession.TryGetSlot(presenceSessionId, srcSlot, out var item)
                || ItemWire602.IsEmpty(item)
                || !PlayerShopSession.IsInventorySlot(srcSlot))
            {
                return (false, Array.Empty<byte>(), null, "empty-inv-source");
            }

            if (!PlayerWarehouseSession.TryGetSessionSlots(presenceSessionId, out var whSlots)
                || !WarehouseBagGrid.CanPlaceAtWithHeal(whSlots, item, dstSlot, ignoreWireSlot: null))
            {
                return (false, Array.Empty<byte>(), null, "warehouse-footprint");
            }

            var moved = item.ToArray();
            PlayerShopSession.SetSlot(presenceSessionId, srcSlot, Array.Empty<byte>());
            PlayerWarehouseSession.SetSlot(presenceSessionId, dstSlot, moved);
            PersistSlotMirror(presenceSessionId, accountId, characterName10, srcSlot);
            PlayerWarehouseSession.PersistSlot(accountId, dstSlot, moved);
            return (true, moved, null, null);
        }

        if (srcFlag == ItemStorageFlags602.Warehouse && dstFlag == ItemStorageFlags602.Inventory)
        {
            await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
                .ConfigureAwait(false);
            await PlayerWarehouseSession.EnsureLoadedAsync(presenceSessionId, accountId, ct).ConfigureAwait(false);
            if (!PlayerWarehouseSession.TryGetSlot(presenceSessionId, srcSlot, out var item)
                || ItemWire602.IsEmpty(item))
            {
                return (false, Array.Empty<byte>(), null, "empty-warehouse-source");
            }

            if (!PlayerShopSession.IsInventorySlot(dstSlot))
            {
                return (false, Array.Empty<byte>(), null, "invalid-inv-dest");
            }

            if (!PlayerShopSession.TryGetSessionSlots(presenceSessionId, out var invSlots)
                || !InventoryBagGrid.CanPlaceAt(invSlots, item, dstSlot, ignoreWireSlot: null))
            {
                return (false, Array.Empty<byte>(), null, "inv-footprint");
            }

            var moved = item.ToArray();
            PlayerWarehouseSession.SetSlot(presenceSessionId, srcSlot, Array.Empty<byte>());
            PlayerShopSession.SetSlot(presenceSessionId, dstSlot, moved);
            PlayerWarehouseSession.PersistSlot(accountId, srcSlot, Array.Empty<byte>());
            PersistSlotMirror(presenceSessionId, accountId, characterName10, dstSlot);
            return (true, moved, null, null);
        }

        if (PlayerTradeSession.TryApplyMove(
                presenceSessionId,
                accountId,
                characterName10,
                srcFlag,
                srcSlot,
                dstFlag,
                dstSlot,
                out var tradeItem))
        {
            return (true, tradeItem, null, null);
        }

        return (false, Array.Empty<byte>(), null, "unsupported-route");
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
        if (!CanUseItems(player, presenceSessionId))
        {
            await writeAsync(ItemWorldWire602.BuildDropFail(slot), ct).ConfigureAwait(false);
            return true;
        }

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
        if (!CanUseItems(player, presenceSessionId))
        {
            await writeAsync(ItemWorldWire602.BuildPickFail(), ct).ConfigureAwait(false);
            return true;
        }

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

        if (ItemWire602.IsZenItem(item12))
        {
            var zenGain = Math.Max(1, (int)item12[2]);
            player.Zen += zenGain;
            onRosterDirty?.Invoke();
            var zenBuf = new byte[ItemWire602.WireBytes];
            ItemWire602.WriteZenPickTotal(zenBuf, player.Zen);
            await writeAsync(ItemWorldWire602.BuildPick(ItemWorldWire602.PickZen, zenBuf), ct).ConfigureAwait(false);
            await writeAsync(ItemViewportWire602.BuildDeleteSingle(mapItemIndex), ct).ConfigureAwait(false);
            Console.WriteLine("[m7] item pick zen +{0} total={1} {2}", zenGain, player.Zen, remote);
            return true;
        }

        if (PlayerShopSession.TryStackIntoBag(presenceSessionId, item12, out var stackSlot)
            && PlayerShopSession.TryGetSlot(presenceSessionId, stackSlot, out var stacked))
        {
            PlayerShopSession.PersistSlotToMirror(accountId, characterName10, stackSlot, stacked);
            await writeAsync(ItemWorldWire602.BuildPick(ItemWorldWire602.PickStack, stacked), ct).ConfigureAwait(false);
            await writeAsync(ItemViewportWire602.BuildDeleteSingle(mapItemIndex), ct).ConfigureAwait(false);
            Console.WriteLine("[m7] item pick stack slot={0} {1}", stackSlot, remote);
            return true;
        }

        if (!PlayerShopSession.TryFindEmptyBagSlot(presenceSessionId, item12, out var bagSlot))
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

    static async Task<bool> HandleUseAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        byte sourceSlotWire,
        byte targetSlotWire,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        string remote,
        CancellationToken ct)
    {
        if (!CanUseItems(player, presenceSessionId))
        {
            return true;
        }

        var sourceSlot = ClientGameplayPackets602.NormalizeItemUseSlot(sourceSlotWire);
        var targetSlot = ClientGameplayPackets602.NormalizeItemUseSlot(targetSlotWire);
        if (!ItemWire602.IsBagSlot(sourceSlot))
        {
            return true;
        }

        await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
            .ConfigureAwait(false);

        if (!PlayerShopSession.TryGetSlot(presenceSessionId, sourceSlot, out var blob) || ItemWire602.IsEmpty(blob))
        {
            return true;
        }

        var itemIndex = ItemWire602.DecodeItemIndex(blob);
        if (targetSlotWire != sourceSlotWire
            && sourceSlot != targetSlot
            && ItemWire602.IsBagSlot(targetSlot)
            && PlayerShopSession.TryGetSlot(presenceSessionId, targetSlot, out var targetBlob)
            && !ItemWire602.IsEmpty(targetBlob)
            && InventoryJewelUseRules.CanRepairFenrirWithBless(itemIndex, targetBlob))
        {
            var empty = new byte[ItemWire602.WireBytes];
            PlayerShopSession.SetSlot(presenceSessionId, sourceSlot, empty);
            PlayerShopSession.PersistSlotToMirror(accountId, characterName10, sourceSlot, empty);
            await writeAsync(ItemWorldWire602.BuildItemDelete(sourceSlot), ct).ConfigureAwait(false);

            ItemWire602.SetDurability(targetBlob, 255);
            PlayerShopSession.SetSlot(presenceSessionId, targetSlot, targetBlob);
            PlayerShopSession.PersistSlotToMirror(accountId, characterName10, targetSlot, targetBlob);
            await writeAsync(ItemWorldWire602.BuildItemDur(targetSlot, 255, 1), ct).ConfigureAwait(false);

            Console.WriteLine(
                "[m7] fenrir repair bless slot={0} → fenrir={1} {2}",
                sourceSlot,
                targetSlot,
                remote);
            return true;
        }

        // Self-use potions send target wire 0; do not compare normalized target (0 → bag slot 12).
        if (targetSlotWire != 0 && sourceSlot == targetSlot)
        {
            return true;
        }

        if (InventorySkillOrbRules.TryGetSkillOrbLearn(itemIndex, out var orbLearn))
        {
            return await HandleSkillOrbUseAsync(
                    player,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    sourceSlot,
                    orbLearn,
                    itemIndex,
                    writeAsync,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        var maxHp = Math.Max(1, player.MaxHp);
        var maxMp = Math.Max(1, player.MaxMp);
        player.MaxShield = InventoryConsumableRules.EnsureMaxShield(maxHp, player.MaxShield);
        var maxShield = player.MaxShield;
        if (!InventoryConsumableRules.TryGetPotionHeal(itemIndex, maxHp, maxMp, maxShield, out var heal))
        {
            Console.WriteLine("[m7] item use unsupported idx={0} slot={1} {2}", itemIndex, sourceSlot, remote);
            return true;
        }

        if (heal.Hp > 0 || heal.Shield > 0)
        {
            if (heal.Hp > 0)
            {
                var cur = player.CurrentHp > 0 ? player.CurrentHp : maxHp;
                player.CurrentHp = Math.Min(maxHp, cur + heal.Hp);
            }

            if (heal.Shield > 0)
            {
                var curSd = player.CurrentShield > 0 ? player.CurrentShield : 0;
                player.CurrentShield = Math.Min(maxShield, curSd + heal.Shield);
            }

            await writeAsync(
                    LifeManaWire602.BuildLife(
                        LifeManaWire602.TypeCurrent,
                        (ushort)Math.Clamp(player.CurrentHp, 0, ushort.MaxValue),
                        (ushort)Math.Clamp(player.CurrentShield, 0, ushort.MaxValue)),
                    ct)
                .ConfigureAwait(false);
        }

        if (heal.Mp > 0)
        {
            var curMp = player.CurrentMp > 0 ? player.CurrentMp : maxMp;
            player.CurrentMp = Math.Min(maxMp, curMp + heal.Mp);
            await writeAsync(
                    LifeManaWire602.BuildMana(
                        LifeManaWire602.TypeCurrent,
                        (ushort)Math.Clamp(player.CurrentMp, 0, ushort.MaxValue)),
                    ct)
                .ConfigureAwait(false);
        }

        onRosterDirty?.Invoke();
        SyncViewerVitals(presenceSessionId, player);
        var charName = Encoding.ASCII.GetString(characterName10.AsSpan(0, 10)).TrimEnd('\0');
        RosterVitalsCombat.ScheduleVitalsMirror(accountId, charName, player);

        var dur = ItemWire602.DecodeDurability(blob);
        if (dur <= 1)
        {
            var empty = new byte[ItemWire602.WireBytes];
            PlayerShopSession.SetSlot(presenceSessionId, sourceSlot, empty);
            PlayerShopSession.PersistSlotToMirror(accountId, characterName10, sourceSlot, empty);
            await writeAsync(ItemWorldWire602.BuildItemDelete(sourceSlot), ct).ConfigureAwait(false);
        }
        else
        {
            var nextDur = (byte)(dur - 1);
            ItemWire602.SetDurability(blob, nextDur);
            PlayerShopSession.SetSlot(presenceSessionId, sourceSlot, blob);
            PlayerShopSession.PersistSlotToMirror(accountId, characterName10, sourceSlot, blob);
            await writeAsync(ItemWorldWire602.BuildItemDur(sourceSlot, nextDur), ct).ConfigureAwait(false);
        }

        Console.WriteLine(
            "[m7] item use potion slot={0} idx={1} hp={2}/{3} mp={4}/{5} {6}",
            sourceSlot,
            itemIndex,
            player.CurrentHp,
            player.MaxHp,
            player.CurrentMp,
            player.MaxMp,
            remote);
        return true;
    }

    static async Task<bool> HandleSkillOrbUseAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        byte sourceSlot,
        InventorySkillOrbRules.SkillOrbLearn orbLearn,
        int itemIndex,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        string remote,
        CancellationToken ct)
    {
        if (!InventorySkillOrbRules.CanCharacterLearn(player.ServerClass, orbLearn))
        {
            Console.WriteLine(
                "[m7] skill orb class mismatch idx={0} skill={1} class=0x{2:X2} {3}",
                itemIndex,
                orbLearn.SkillType,
                player.ServerClass,
                remote);
            return true;
        }

        if (player.Level < orbLearn.MinLevel)
        {
            Console.WriteLine(
                "[m7] skill orb level too low idx={0} skill={1} need={2} have={3} {4}",
                itemIndex,
                orbLearn.SkillType,
                orbLearn.MinLevel,
                player.Level,
                remote);
            return true;
        }

        var charName = Encoding.ASCII.GetString(characterName10.AsSpan(0, 10)).TrimEnd('\0');
        var skillRepo = TakumiPostgresMirror.CharacterSkills;
        var rows = new List<CharacterSkillRow>();
        if (skillRepo is not null && !string.IsNullOrEmpty(accountId))
        {
            var loaded = await skillRepo
                .LoadByCharacterAsync(accountId, charName, ct)
                .ConfigureAwait(false);
            rows.AddRange(loaded);
        }

        if (rows.Any(r => r.Type == orbLearn.SkillType))
        {
            var existing = rows.First(r => r.Type == orbLearn.SkillType);
            await writeAsync(
                    MagicListWire602.BuildAddSkill(existing.Slot, orbLearn.SkillType, existing.Level),
                    ct)
                .ConfigureAwait(false);
            Console.WriteLine(
                "[m7] skill orb already learned skill={0} (resent F3 FE sync) {1}",
                orbLearn.SkillType,
                remote);
            return true;
        }

        var skillSlot = InventorySkillOrbRules.PickSkillSlot(orbLearn.SkillType, rows);
        rows.Add(new CharacterSkillRow
        {
            Slot = skillSlot,
            Type = orbLearn.SkillType,
            Level = 1,
        });

        if (skillRepo is not null && !string.IsNullOrEmpty(accountId))
        {
            await skillRepo.ReplaceAllAsync(accountId, charName, rows, ct).ConfigureAwait(false);
        }

        var empty = new byte[ItemWire602.WireBytes];
        PlayerShopSession.SetSlot(presenceSessionId, sourceSlot, empty);
        PlayerShopSession.PersistSlotToMirror(accountId, characterName10, sourceSlot, empty);
        await writeAsync(ItemWorldWire602.BuildItemDelete(sourceSlot), ct).ConfigureAwait(false);
        await writeAsync(MagicListWire602.BuildAddSkill(skillSlot, orbLearn.SkillType, 1), ct)
            .ConfigureAwait(false);

        Console.WriteLine(
            "[m7] skill orb learned skill={0} slot={1} inv={2} idx={3} {4}",
            orbLearn.SkillType,
            skillSlot,
            sourceSlot,
            itemIndex,
            remote);
        return true;
    }

    static void SyncViewerVitals(Guid presenceSessionId, GameRosterEntry player)
    {
        if (!MonsterViewerRegistry.TryGetSession(presenceSessionId, out var session))
        {
            return;
        }

        session.CurrentHp = player.CurrentHp;
        session.MaxHp = player.MaxHp;
        session.CurrentMp = player.CurrentMp;
        session.MaxMp = player.MaxMp;
        session.OnVitalsChanged?.Invoke(player.CurrentHp, player.MaxHp);
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

    static async Task PersistInventorySnapshotAsync(
        Guid sessionId,
        string? accountId,
        byte[] characterName10,
        CancellationToken ct)
    {
        if (string.IsNullOrEmpty(accountId)
            || !PlayerShopSession.TryGetSessionSlots(sessionId, out var slots))
        {
            return;
        }

        await InventorySlotPersist.SaveSlotsAsync(accountId, characterName10, slots, ct).ConfigureAwait(false);
    }
}
