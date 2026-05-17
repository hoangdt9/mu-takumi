using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>8×15 warehouse pages (wire slots 0–119 main, 120–239 extended).</summary>
public static class WarehouseBagGrid
{
    public const int Columns = 8;
    public const int Rows = 15;
    public const int CellsPerPage = Columns * Rows;
    public const byte MainPageLastSlot = 119;
    public const byte ExtPageFirstSlot = 120;

    public static bool IsWarehouseSlot(byte slot) => slot <= ItemStorageFlags602.MaxWarehouseSlot;

    public static bool WireToCell(byte wireSlot, out int col, out int row)
    {
        col = -1;
        row = -1;
        if (!IsWarehouseSlot(wireSlot))
        {
            return false;
        }

        var linear = wireSlot <= MainPageLastSlot ? wireSlot : wireSlot - ExtPageFirstSlot;
        if (linear < 0 || linear >= CellsPerPage)
        {
            return false;
        }

        col = linear % Columns;
        row = linear / Columns;
        return true;
    }

    public static byte CellToWire(byte pageFirstSlot, int col, int row) =>
        (byte)(pageFirstSlot + (row * Columns) + col);

    public static byte PageFirstSlot(byte wireSlot) =>
        wireSlot <= MainPageLastSlot ? (byte)0 : ExtPageFirstSlot;

    static bool IsOnPage(byte wireSlot, byte pageFirstSlot) =>
        pageFirstSlot == 0
            ? wireSlot <= MainPageLastSlot
            : wireSlot >= ExtPageFirstSlot;

    public static bool CanPlaceAt(
        IReadOnlyDictionary<byte, byte[]> slots,
        ReadOnlySpan<byte> item12,
        byte targetWireSlot,
        byte? ignoreWireSlot)
    {
        if (!WireToCell(targetWireSlot, out var col, out var row))
        {
            return false;
        }

        ItemSizeCatalog.GetSize(item12, out var w, out var h);
        var page = PageFirstSlot(targetWireSlot);
        var occupied = BuildOccupancy(slots, ignoreWireSlot, page);
        return IsRegionFree(occupied, col, row, w, h);
    }

    /// <summary>Repack stale DB anchors once, then re-check placement (heals ghost footprints blocking drag targets).</summary>
    public static bool CanPlaceAtWithHeal(
        Dictionary<byte, byte[]> slots,
        ReadOnlySpan<byte> item12,
        byte targetWireSlot,
        byte? ignoreWireSlot)
    {
        if (CanPlaceAt(slots, item12, targetWireSlot, ignoreWireSlot))
        {
            return true;
        }

        return CompactWarehouseSlots(slots)
               && CanPlaceAt(slots, item12, targetWireSlot, ignoreWireSlot);
    }

    public static bool TryFindEmptyAnchor(
        IReadOnlyDictionary<byte, byte[]> slots,
        byte pageFirstSlot,
        ReadOnlySpan<byte> item12,
        out byte wireSlot)
    {
        wireSlot = 0;
        ItemSizeCatalog.GetSize(item12, out var w, out var h);
        var occupied = BuildOccupancy(slots, excludeSlot: null, pageFirstSlot);
        for (var row = 0; row <= Rows - h; row++)
        {
            for (var col = 0; col <= Columns - w; col++)
            {
                if (col + w > Columns)
                {
                    continue;
                }

                if (!IsRegionFree(occupied, col, row, w, h))
                {
                    continue;
                }

                wireSlot = CellToWire(pageFirstSlot, col, row);
                return true;
            }
        }

        return false;
    }

    /// <summary>Prune invalid keys, dedupe identical blobs, repack each 8×15 page (heals stale DB anchors).</summary>
    public static bool CompactWarehouseSlots(IDictionary<byte, byte[]> slots)
    {
        PruneWarehouseSlots(slots);
        var mainItems = CollectPageItems(slots, pageFirstSlot: 0);
        var extItems = CollectPageItems(slots, pageFirstSlot: ExtPageFirstSlot);
        var keysBefore = slots.Keys.OrderBy(static x => x).ToArray();
        slots.Clear();
        RepackPage(slots, 0, mainItems);
        RepackPage(slots, ExtPageFirstSlot, extItems);
        var keysAfter = slots.Keys.OrderBy(static x => x).ToArray();
        return keysBefore.Length != keysAfter.Length || !keysBefore.AsSpan().SequenceEqual(keysAfter);
    }

    static void PruneWarehouseSlots(IDictionary<byte, byte[]> slots)
    {
        var remove = new List<byte>();
        foreach (var kv in slots)
        {
            if (ItemWire602.IsEmpty(kv.Value)
                || !IsWarehouseSlot(kv.Key)
                || ItemWire602.DecodeItemIndex(kv.Value) < 0)
            {
                remove.Add(kv.Key);
            }
        }

        foreach (var key in remove)
        {
            slots.Remove(key);
        }
    }

    static List<byte[]> CollectPageItems(IDictionary<byte, byte[]> slots, byte pageFirstSlot)
    {
        var items = new List<byte[]>();
        foreach (var kv in slots)
        {
            if (!IsOnPage(kv.Key, pageFirstSlot) || ItemWire602.IsEmpty(kv.Value))
            {
                continue;
            }

            var copy = kv.Value.ToArray();
            if (items.All(existing => !existing.AsSpan().SequenceEqual(copy)))
            {
                items.Add(copy);
            }
        }

        items.Sort(static (a, b) =>
        {
            ItemSizeCatalog.GetSize(b, out var wb, out var hb);
            ItemSizeCatalog.GetSize(a, out var wa, out var ha);
            var areaCmp = (wb * hb).CompareTo(wa * ha);
            return areaCmp != 0 ? areaCmp : ItemWire602.DecodeItemIndex(a).CompareTo(ItemWire602.DecodeItemIndex(b));
        });
        return items;
    }

    static void RepackPage(IDictionary<byte, byte[]> slots, byte pageFirstSlot, List<byte[]> items)
    {
        var repacked = new Dictionary<byte, byte[]>();
        foreach (var item12 in items)
        {
            if (!TryFindEmptyAnchor(repacked, pageFirstSlot, item12, out var anchor))
            {
                Console.WriteLine(
                    "[warehouse] repack dropped item idx={0} (no footprint on page {1})",
                    ItemWire602.DecodeItemIndex(item12),
                    pageFirstSlot);
                continue;
            }

            repacked[anchor] = item12.ToArray();
        }

        foreach (var kv in repacked)
        {
            slots[kv.Key] = kv.Value;
        }
    }

    static bool[] BuildOccupancy(IReadOnlyDictionary<byte, byte[]> slots, byte? excludeSlot, byte pageFirstSlot)
    {
        var occupied = new bool[CellsPerPage];
        foreach (var kv in slots)
        {
            if (ItemWire602.IsEmpty(kv.Value) || !IsOnPage(kv.Key, pageFirstSlot))
            {
                continue;
            }

            if (excludeSlot is { } ex && kv.Key == ex)
            {
                continue;
            }

            if (!WireToCell(kv.Key, out var col, out var row))
            {
                continue;
            }

            ItemSizeCatalog.GetSize(kv.Value, out var w, out var h);
            MarkRegion(occupied, col, row, w, h);
        }

        return occupied;
    }

    static bool IsRegionFree(bool[] occupied, int col, int row, int width, int height)
    {
        for (var y = 0; y < height; y++)
        {
            for (var x = 0; x < width; x++)
            {
                var c = col + x;
                var r = row + y;
                if (c < 0 || c >= Columns || r < 0 || r >= Rows)
                {
                    return false;
                }

                if (occupied[(r * Columns) + c])
                {
                    return false;
                }
            }
        }

        return true;
    }

    static void MarkRegion(bool[] occupied, int col, int row, int width, int height)
    {
        for (var y = 0; y < height; y++)
        {
            for (var x = 0; x < width; x++)
            {
                occupied[((row + y) * Columns) + (col + x)] = true;
            }
        }
    }
}
