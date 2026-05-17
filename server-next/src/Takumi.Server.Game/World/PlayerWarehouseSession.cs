using System.Collections.Concurrent;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Per-connection warehouse UI state + in-memory grid (parity <c>WAREHOUSE_SIZE</c> 240).</summary>
public static class PlayerWarehouseSession
{
    static readonly ConcurrentDictionary<Guid, WarehouseState> Sessions = new();

    public readonly record struct WarehouseState(bool IsOpen, Dictionary<byte, byte[]> Slots);

    public static void Open(Guid sessionId)
    {
        PlayerUiSession.SetWarehouse(sessionId, true);
        Sessions.AddOrUpdate(
            sessionId,
            _ => new WarehouseState(true, new Dictionary<byte, byte[]>()),
            (_, existing) => existing with { IsOpen = true });
    }

    public static void Close(Guid sessionId)
    {
        PlayerUiSession.SetWarehouse(sessionId, false);
        if (Sessions.TryGetValue(sessionId, out var s))
        {
            Sessions[sessionId] = s with { IsOpen = false };
        }
    }

    public static bool IsOpen(Guid sessionId) =>
        Sessions.TryGetValue(sessionId, out var s) && s.IsOpen;

    public static async Task EnsureLoadedAsync(Guid sessionId, string? accountLogin, CancellationToken ct)
    {
        var s = Sessions.GetOrAdd(sessionId, _ => new WarehouseState(false, new Dictionary<byte, byte[]>()));
        if (s.Slots.Count > 0 || string.IsNullOrEmpty(accountLogin))
        {
            return;
        }

        await LoadFromMirrorAsync(s, accountLogin, ct).ConfigureAwait(false);
    }

    /// <summary>Reload from DB, repack footprints, persist healed layout (call on each warehouse open).</summary>
    public static async Task ReloadAndHealForOpenAsync(
        Guid sessionId,
        string? accountLogin,
        CancellationToken ct)
    {
        var s = Sessions.GetOrAdd(sessionId, _ => new WarehouseState(false, new Dictionary<byte, byte[]>()));
        s.Slots.Clear();
        if (!string.IsNullOrEmpty(accountLogin))
        {
            await LoadFromMirrorAsync(s, accountLogin, ct).ConfigureAwait(false);
        }

        if (s.Slots.Count == 0)
        {
            return;
        }

        ItemSizeCatalog.EnsureInitialized();
        var anchorsBefore = s.Slots.Keys.OrderBy(static x => x).ToArray();
        var healed = WarehouseBagGrid.CompactWarehouseSlots(s.Slots);
        if (!healed)
        {
            return;
        }

        var anchorsAfter = s.Slots.Keys.OrderBy(static x => x).ToArray();
        Console.WriteLine(
            "[warehouse] repack healed account={0} anchors [{1}] → [{2}]",
            accountLogin,
            string.Join(',', anchorsBefore),
            string.Join(',', anchorsAfter));

        if (!string.IsNullOrEmpty(accountLogin) && TakumiPostgresMirror.WarehouseSlots is not null)
        {
            WarehouseSlotMirrorWriter.ScheduleReplaceAccount(accountLogin, BuildSnapshot(sessionId));
        }
    }

    static async Task LoadFromMirrorAsync(WarehouseState s, string accountLogin, CancellationToken ct)
    {
        var repo = TakumiPostgresMirror.WarehouseSlots;
        if (repo is null)
        {
            return;
        }

        try
        {
            var rows = await repo.LoadByAccountAsync(accountLogin, ct).ConfigureAwait(false);
            foreach (var row in rows)
            {
                s.Slots[row.Slot] = row.Item12.ToArray();
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine("[warehouse] load failed account={0}: {1}", accountLogin, ex.Message);
        }
    }

    public static bool TryGetSlot(Guid sessionId, byte slot, out byte[] item12)
    {
        item12 = Array.Empty<byte>();
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            return false;
        }

        return s.Slots.TryGetValue(slot, out item12!);
    }

    public static void SetSlot(Guid sessionId, byte slot, byte[] item12)
    {
        var s = Sessions.GetOrAdd(sessionId, _ => new WarehouseState(false, new Dictionary<byte, byte[]>()));
        if (ItemWire602.IsEmpty(item12))
        {
            s.Slots.Remove(slot);
        }
        else
        {
            s.Slots[slot] = item12;
        }
    }

    public static bool TryGetSessionSlots(Guid sessionId, out Dictionary<byte, byte[]> slots)
    {
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            slots = new Dictionary<byte, byte[]>();
            return false;
        }

        slots = s.Slots;
        return true;
    }

    public static bool TryMoveSlot(Guid sessionId, byte sourceSlot, byte targetSlot, out byte[] targetItem12)
    {
        targetItem12 = Array.Empty<byte>();
        if (sourceSlot == targetSlot
            || !WarehouseBagGrid.IsWarehouseSlot(sourceSlot)
            || !WarehouseBagGrid.IsWarehouseSlot(targetSlot))
        {
            return false;
        }

        var s = Sessions.GetOrAdd(sessionId, _ => new WarehouseState(false, new Dictionary<byte, byte[]>()));
        s.Slots.TryGetValue(sourceSlot, out var sourceItem);
        sourceItem ??= Array.Empty<byte>();
        if (ItemWire602.IsEmpty(sourceItem))
        {
            return false;
        }

        s.Slots.TryGetValue(targetSlot, out var destItem);
        destItem ??= Array.Empty<byte>();
        var moved = sourceItem.ToArray();
        if (ItemWire602.IsEmpty(destItem))
        {
            if (!WarehouseBagGrid.CanPlaceAtWithHeal(s.Slots, moved, targetSlot, ignoreWireSlot: sourceSlot))
            {
                return false;
            }

            s.Slots.Remove(sourceSlot);
            s.Slots[targetSlot] = moved;
        }
        else
        {
            if (!WarehouseBagGrid.CanPlaceAtWithHeal(s.Slots, moved, targetSlot, ignoreWireSlot: sourceSlot)
                || !WarehouseBagGrid.CanPlaceAtWithHeal(s.Slots, destItem, sourceSlot, ignoreWireSlot: targetSlot))
            {
                return false;
            }

            s.Slots[sourceSlot] = destItem.ToArray();
            s.Slots[targetSlot] = moved;
        }

        targetItem12 = moved;
        return true;
    }

    public static IReadOnlyList<InventorySlotRow> BuildSnapshot(Guid sessionId)
    {
        if (!Sessions.TryGetValue(sessionId, out var s) || s.Slots.Count == 0)
        {
            return Array.Empty<InventorySlotRow>();
        }

        var list = new List<InventorySlotRow>(s.Slots.Count);
        foreach (var kv in s.Slots)
        {
            if (!ItemWire602.IsEmpty(kv.Value))
            {
                list.Add(new InventorySlotRow { Slot = kv.Key, Item12 = kv.Value });
            }
        }

        return list;
    }

    public static void PersistSlot(string? accountLogin, byte slot, byte[] item12)
    {
        if (string.IsNullOrEmpty(accountLogin) || TakumiPostgresMirror.WarehouseSlots is null)
        {
            return;
        }

        if (ItemWire602.IsEmpty(item12))
        {
            WarehouseSlotMirrorWriter.ScheduleDeleteSlot(accountLogin, slot);
        }
        else
        {
            WarehouseSlotMirrorWriter.ScheduleUpsertSlot(accountLogin, slot, item12);
        }
    }

    public static void FlushOnDisconnect(string? accountLogin, Guid sessionId)
    {
        if (string.IsNullOrEmpty(accountLogin))
        {
            Sessions.TryRemove(sessionId, out _);
            return;
        }

        var snapshot = BuildSnapshot(sessionId);
        if (snapshot.Count > 0 && TakumiPostgresMirror.WarehouseSlots is not null)
        {
            WarehouseSlotMirrorWriter.ScheduleReplaceAccount(accountLogin, snapshot);
        }

        Sessions.TryRemove(sessionId, out _);
    }
}
