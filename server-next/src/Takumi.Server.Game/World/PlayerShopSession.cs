using System.Collections.Concurrent;
using System.Text;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Per-connection NPC shop + bag inventory; mirrors to <c>inventory_slot</c> when DB sync is on.</summary>
public static class PlayerShopSession
{
    static readonly ConcurrentDictionary<Guid, SessionState> Sessions = new();

    public readonly record struct SessionState(int ShopIndex, byte? PendingBuySlot, Dictionary<byte, byte[]> Slots);

    public static void OpenShop(Guid sessionId, int shopIndex) =>
        Sessions.AddOrUpdate(
            sessionId,
            _ => new SessionState(shopIndex, null, new Dictionary<byte, byte[]>()),
            (_, existing) => existing with { ShopIndex = shopIndex, PendingBuySlot = null });

    public static void SetPendingBuy(Guid sessionId, byte shopSlot)
    {
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            return;
        }

        Sessions[sessionId] = s with { PendingBuySlot = shopSlot };
    }

    public static void ClearPendingBuy(Guid sessionId)
    {
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            return;
        }

        Sessions[sessionId] = s with { PendingBuySlot = null };
    }

    public static bool TryGetPendingBuy(Guid sessionId, out byte shopSlot)
    {
        if (Sessions.TryGetValue(sessionId, out var s) && s.PendingBuySlot is { } slot)
        {
            shopSlot = slot;
            return true;
        }

        shopSlot = 0;
        return false;
    }

    public static void CloseShop(Guid sessionId)
    {
        if (Sessions.TryGetValue(sessionId, out var s))
        {
            Sessions[sessionId] = s with { ShopIndex = -1, PendingBuySlot = null };
        }
    }

    public static bool TryGetShopIndex(Guid sessionId, out int shopIndex)
    {
        if (Sessions.TryGetValue(sessionId, out var s) && s.ShopIndex >= 0)
        {
            shopIndex = s.ShopIndex;
            return true;
        }

        shopIndex = -1;
        return false;
    }

    public static void ReplaceSlots(Guid sessionId, IReadOnlyDictionary<byte, byte[]> slots)
    {
        var copy = new Dictionary<byte, byte[]>(slots.Count);
        foreach (var kv in slots)
        {
            if (kv.Value.Length == ItemWire602.WireBytes && !ItemWire602.IsEmpty(kv.Value))
            {
                copy[kv.Key] = kv.Value.ToArray();
            }
        }

        Sessions.AddOrUpdate(
            sessionId,
            _ => new SessionState(-1, null, copy),
            (_, existing) => existing with { Slots = copy });
    }

    public static void ClearSlots(Guid sessionId)
    {
        if (Sessions.TryGetValue(sessionId, out var s))
        {
            s.Slots.Clear();
        }
    }

    public static async Task EnsureInventoryLoadedAsync(
        Guid sessionId,
        string? accountId,
        byte[] characterName10,
        CancellationToken ct)
    {
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            s = new SessionState(-1, null, new Dictionary<byte, byte[]>());
            Sessions[sessionId] = s;
        }

        if (s.Slots.Count > 0)
        {
            return;
        }

        var rows = await JoinInventoryPacket602
            .LoadRowsAsync(TakumiPostgresMirror.InventorySlots, accountId, characterName10, ct)
            .ConfigureAwait(false);
        foreach (var row in rows)
        {
            s.Slots[row.Slot] = row.Item12.ToArray();
        }

        if (s.Slots.Count > 0)
        {
            InventoryBagGrid.RepackBagSlots(s.Slots);
        }
    }

    public static bool TryGetSlot(Guid sessionId, byte slot, out byte[] item12)
    {
        item12 = Array.Empty<byte>();
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            return false;
        }

        if (s.Slots.TryGetValue(slot, out var blob))
        {
            item12 = blob;
            return true;
        }

        return false;
    }

    public static void SetSlot(Guid sessionId, byte slot, byte[] item12)
    {
        var s = Sessions.GetOrAdd(sessionId, _ => new SessionState(-1, null, new Dictionary<byte, byte[]>()));
        if (ItemWire602.IsEmpty(item12))
        {
            s.Slots.Remove(slot);
        }
        else
        {
            s.Slots[slot] = item12;
        }
    }

    /// <summary>Mirror one slot to <c>inventory_slot</c> when <c>TAKUMI_ROSTER_DB_SYNC</c> is on.</summary>
    public static void PersistSlotToMirror(string? accountId, byte[] characterName10, byte slot, byte[] item12)
    {
        if (string.IsNullOrEmpty(accountId) || TakumiPostgresMirror.InventorySlots is null)
        {
            return;
        }

        var charName = CharacterRosterMerge.NormaliseName(Encoding.ASCII.GetString(characterName10.AsSpan(0, 10)));
        if (charName.Length == 0)
        {
            return;
        }

        if (ItemWire602.IsEmpty(item12))
        {
            InventorySlotMirrorWriter.ScheduleDeleteSlot(accountId, charName, slot);
        }
        else
        {
            InventorySlotMirrorWriter.ScheduleUpsertSlot(accountId, charName, slot, item12);
        }
    }

    public static IReadOnlyList<InventorySlotRow> BuildSlotSnapshot(Guid sessionId)
    {
        if (!Sessions.TryGetValue(sessionId, out var s) || s.Slots.Count == 0)
        {
            return Array.Empty<InventorySlotRow>();
        }

        var list = new List<InventorySlotRow>(s.Slots.Count);
        foreach (var kv in s.Slots)
        {
            if (ItemWire602.IsEmpty(kv.Value))
            {
                continue;
            }

            list.Add(new InventorySlotRow { Slot = kv.Key, Item12 = kv.Value });
        }

        return list;
    }

    public static void FlushInventoryMirrorOnDisconnect(string? accountId, byte[]? characterName10, Guid sessionId)
    {
        if (string.IsNullOrEmpty(accountId) || characterName10 is null || characterName10.Length < 10)
        {
            RemoveSession(sessionId);
            return;
        }

        var snapshot = BuildSlotSnapshot(sessionId);
        if (snapshot.Count > 0 && TakumiPostgresMirror.InventorySlots is not null)
        {
            var charName = CharacterRosterMerge.NormaliseName(Encoding.ASCII.GetString(characterName10.AsSpan(0, 10)));
            if (charName.Length > 0)
            {
                InventorySlotMirrorWriter.ScheduleReplaceCharacter(accountId, charName, snapshot);
            }
        }

        RemoveSession(sessionId);
    }

    public static bool IsInventorySlot(byte slot) => slot <= ItemWire602.LastBagSlot;

    /// <summary>Move or swap between inventory slots (flags 0→0).</summary>
    public static bool TryMoveInventorySlot(
        Guid sessionId,
        byte sourceSlot,
        byte targetSlot,
        out byte[] targetItem12,
        out byte[]? swappedIntoSource,
        out string? failReason)
    {
        targetItem12 = Array.Empty<byte>();
        swappedIntoSource = null;
        failReason = null;
        if (sourceSlot == targetSlot
            || sourceSlot > ItemWire602.LastBagSlot
            || targetSlot > ItemWire602.LastBagSlot)
        {
            failReason = "invalid-slot";
            return false;
        }

        var s = Sessions.GetOrAdd(sessionId, _ => new SessionState(-1, null, new Dictionary<byte, byte[]>()));
        s.Slots.TryGetValue(sourceSlot, out var sourceItem);
        sourceItem ??= Array.Empty<byte>();
        if (ItemWire602.IsEmpty(sourceItem))
        {
            failReason = "empty-source";
            return false;
        }

        if (!InventoryEquipRules.CanMoveBetweenSlots(sourceSlot, targetSlot, sourceItem))
        {
            failReason = "equip-rule";
            return false;
        }

        s.Slots.TryGetValue(targetSlot, out var destItem);
        destItem ??= Array.Empty<byte>();

        if (ItemWire602.IsBagSlot(sourceSlot)
            && ItemWire602.IsBagSlot(targetSlot)
            && ItemWire602.IsEmpty(destItem)
            && !InventoryBagGrid.CanPlaceAt(s.Slots, sourceItem, targetSlot, sourceSlot))
        {
            ItemSizeCatalog.GetSize(sourceItem, out var w, out var h);
            failReason = $"footprint-blocked size={w}x{h}";
            return false;
        }
        var moved = sourceItem.ToArray();
        if (ItemWire602.IsEmpty(destItem))
        {
            s.Slots.Remove(sourceSlot);
            s.Slots[targetSlot] = moved;
        }
        else
        {
            swappedIntoSource = destItem.ToArray();
            s.Slots[sourceSlot] = swappedIntoSource;
            s.Slots[targetSlot] = moved;
        }

        targetItem12 = s.Slots[targetSlot];
        return true;
    }

    public static bool TryStackIntoBag(Guid sessionId, byte[] item12, out byte resultSlot)
    {
        resultSlot = 0;
        if (ItemWire602.IsEmpty(item12) || ItemWire602.IsZenItem(item12))
        {
            return false;
        }

        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            return false;
        }

        foreach (var kv in s.Slots)
        {
            if (!ItemWire602.IsBagSlot(kv.Key) || !ItemWire602.ItemsStackable(kv.Value, item12))
            {
                continue;
            }

            var merged = kv.Value.ToArray();
            var sum = Math.Min(255, (int)merged[2] + Math.Max(1, (int)item12[2]));
            ItemWire602.SetDurability(merged, (byte)sum);
            s.Slots[kv.Key] = merged;
            resultSlot = kv.Key;
            return true;
        }

        return false;
    }

    public static bool TryFindEmptyBagSlot(Guid sessionId, out byte slot)
    {
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            slot = 0;
            return false;
        }

        var probe = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(probe, 0, 0, 0, 1, false, false, 0, 0);
        return InventoryBagGrid.TryFindEmptyAnchor(s.Slots, probe, out slot);
    }

    public static bool TryFindEmptyBagSlot(Guid sessionId, ReadOnlySpan<byte> item12, out byte slot)
    {
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            slot = 0;
            return false;
        }

        ItemSizeCatalog.EnsureInitialized();
        return InventoryBagGrid.TryFindEmptyAnchor(s.Slots, item12, out slot);
    }

    /// <summary>Prune, merge stacks, repack bag before shop buy / F3 10 (parity OpenMU CheckInvSpace).</summary>
    public static void CompactBagForPlacement(Guid sessionId)
    {
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            return;
        }

        ItemSizeCatalog.EnsureInitialized();
        var dropped = InventoryBagGrid.CompactBagSlots(s.Slots);
        if (dropped > 0)
        {
            Console.WriteLine("[m8] CompactBagForPlacement: removed {0} orphan anchor(s)", dropped);
        }
    }

    public static void RemoveSession(Guid sessionId) => Sessions.TryRemove(sessionId, out _);

    public static void RepackLoadedBag(Guid sessionId)
    {
        if (Sessions.TryGetValue(sessionId, out var s) && s.Slots.Count > 0)
        {
            InventoryBagGrid.RepackBagSlots(s.Slots);
        }
    }

    /// <summary>Full <c>F3 10</c> inventory list for client resync after shop/repack.</summary>
    public static byte[] BuildInventoryListPacket(Guid sessionId)
    {
        if (!Sessions.TryGetValue(sessionId, out var s) || s.Slots.Count == 0)
        {
            return InventoryListWire602.BuildEmpty();
        }

        ItemSizeCatalog.EnsureInitialized();
        var copy = new Dictionary<byte, byte[]>(s.Slots.Count);
        foreach (var kv in s.Slots)
        {
            if (!ItemWire602.IsEmpty(kv.Value))
            {
                copy[kv.Key] = kv.Value.ToArray();
            }
        }

        JoinInventoryLifecycle.PruneInvalidSlots(copy);
        return JoinInventoryLifecycle.BuildPacketFromSlots(copy);
    }

    public static bool TryGetSessionSlots(Guid sessionId, out IReadOnlyDictionary<byte, byte[]> slots)
    {
        if (Sessions.TryGetValue(sessionId, out var s))
        {
            slots = s.Slots;
            return true;
        }

        slots = new Dictionary<byte, byte[]>();
        return false;
    }

    public static async Task PersistAsync(
        Guid sessionId,
        string? accountId,
        byte[] characterName10,
        long zen,
        CancellationToken ct)
    {
        if (string.IsNullOrWhiteSpace(accountId) || !Sessions.TryGetValue(sessionId, out var s))
        {
            return;
        }

        await InventorySlotPersist.SaveSlotsAsync(accountId, characterName10, s.Slots, ct).ConfigureAwait(false);
        await InventorySlotPersist.SaveZenAsync(accountId, characterName10, zen, ct).ConfigureAwait(false);
        Console.WriteLine(
            "[m8] shop persist slots={0} zen={1} char={2}",
            s.Slots.Count,
            zen,
            Encoding.ASCII.GetString(characterName10).TrimEnd('\0'));
    }
}
