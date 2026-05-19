using System.Text;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Join-time inventory: load DB → repack bag → mirror snapshot → F3 10.</summary>
public static class JoinInventoryLifecycle
{
    public static async Task<byte[]> BuildJoinPacketAsync(
        PostgresInventorySlotRepository? inventoryRepo,
        string? accountLogin,
        byte[] characterName10,
        Guid presenceSessionId,
        CancellationToken ct)
    {
        var rows = await JoinInventoryPacket602
            .LoadRowsAsync(inventoryRepo, accountLogin, characterName10, ct)
            .ConfigureAwait(false);
        if (rows.Count == 0)
        {
            PlayerShopSession.ClearSlots(presenceSessionId);
            return InventoryListWire602.BuildEmpty();
        }

        ItemSizeCatalog.EnsureInitialized();
        var slots = new Dictionary<byte, byte[]>(rows.Count);
        foreach (var row in rows)
        {
            slots[row.Slot] = row.Item12.ToArray();
        }

        ItemWireSanitizer.NormalizeSocketEncoding(slots);

        var keysBefore = rows.Select(static r => r.Slot).OrderBy(static x => x).ToArray();
        InventoryBagGrid.CompactBagSlots(slots);
        var keysAfter = slots.Keys.OrderBy(static x => x).ToArray();
        var layoutChanged = keysBefore.Length != keysAfter.Length || !keysBefore.AsSpan().SequenceEqual(keysAfter);

        if (InventorySlotPersist.IsEnabled && inventoryRepo is not null && !string.IsNullOrEmpty(accountLogin))
        {
            await InventorySlotPersist.SaveSlotsAsync(accountLogin, characterName10, slots, ct).ConfigureAwait(false);
            if (layoutChanged)
            {
                var charName = Encoding.ASCII.GetString(characterName10.AsSpan(0, 10)).TrimEnd('\0');
                Console.WriteLine(
                    "[m8] inventory repack persisted char={0} slots={1} (healed bag anchors)",
                    charName,
                    slots.Count);
            }
        }

        PlayerShopSession.ReplaceSlots(presenceSessionId, slots);
        return BuildPacketFromSlots(slots);
    }

    /// <summary>Season 6 <c>C4 F3 10</c> from current slot map (sorted by wire index).</summary>
    public static byte[] BuildPacketFromSlots(IReadOnlyDictionary<byte, byte[]> slots)
    {
        if (slots.Count == 0)
        {
            return InventoryListWire602.BuildEmpty();
        }

        var buf = new byte[slots.Count * 13];
        var o = 0;
        foreach (var kv in slots.OrderBy(static kv => kv.Key))
        {
            buf[o++] = kv.Key;
            kv.Value.AsSpan(0, InventoryListWire602.ItemWireBytes).CopyTo(buf.AsSpan(o, InventoryListWire602.ItemWireBytes));
            o += InventoryListWire602.ItemWireBytes;
        }

        return InventoryListWire602.Build(buf);
    }

    internal static void PruneInvalidSlots(IDictionary<byte, byte[]> slots)
    {
        var remove = new List<byte>();
        foreach (var kv in slots)
        {
            if (ItemWire602.IsEmpty(kv.Value)
                || ItemWire602.DecodeItemIndex(kv.Value) < 0
                || (!ItemWire602.IsWearSlot(kv.Key) && !ItemWire602.IsBagSlot(kv.Key)))
            {
                remove.Add(kv.Key);
            }
        }

        foreach (var key in remove)
        {
            slots.Remove(key);
        }
    }
}
