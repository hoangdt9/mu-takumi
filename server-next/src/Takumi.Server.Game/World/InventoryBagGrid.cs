using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>8×8 main bag (wire slots <see cref="ItemWire602.FirstBagSlot"/>–<see cref="ItemWire602.LastBagSlot"/>).</summary>
public static class InventoryBagGrid
{
    public const int Columns = 8;
    public const int Rows = 8;
    public const int CellCount = Columns * Rows;

    public static bool WireToCell(byte wireSlot, out int col, out int row)
    {
        if (!ItemWire602.IsBagSlot(wireSlot))
        {
            col = -1;
            row = -1;
            return false;
        }

        var linear = wireSlot - ItemWire602.FirstBagSlot;
        col = linear % Columns;
        row = linear / Columns;
        return true;
    }

    public static byte CellToWire(int col, int row) =>
        (byte)(ItemWire602.FirstBagSlot + (row * Columns) + col);

    public static bool TryFindEmptyAnchor(IReadOnlyDictionary<byte, byte[]> slots, ReadOnlySpan<byte> item12, out byte wireSlot)
    {
        ItemSizeCatalog.GetSize(item12, out var w, out var h);
        var occupied = BuildOccupancy(slots, excludeSlot: null);
        for (var row = 0; row <= Rows - h; row++)
        {
            for (var col = 0; col <= Columns - w; col++)
            {
                // Parity OpenMU Storage.CheckInvSpace — skip anchors that wrap past row width.
                if (col + w > Columns)
                {
                    continue;
                }

                if (!IsRegionFree(occupied, col, row, w, h))
                {
                    continue;
                }

                wireSlot = CellToWire(col, row);
                return true;
            }
        }

        wireSlot = 0;
        return false;
    }

    /// <summary>Merge stackable bag blobs, prune invalid keys, repack top-left (OpenMU CheckInvSpace order).</summary>
    public static int CompactBagSlots(IDictionary<byte, byte[]> slots)
    {
        JoinInventoryLifecycle.PruneInvalidSlots(slots);
        MergeStackableBagItems(slots);
        var before = CountBagAnchors(slots);
        RepackBagSlots(slots);
        var after = CountBagAnchors(slots);
        return before - after;
    }

    public static int CountOccupiedBagCells(IEnumerable<KeyValuePair<byte, byte[]>> slots)
    {
        var map = slots as IReadOnlyDictionary<byte, byte[]>
                  ?? slots.ToDictionary(static kv => kv.Key, static kv => kv.Value);
        var occupied = BuildOccupancy(map, excludeSlot: null);
        var count = 0;
        foreach (var used in occupied)
        {
            if (used)
            {
                count++;
            }
        }

        return count;
    }

    static int CountBagAnchors(IEnumerable<KeyValuePair<byte, byte[]>> slots)
    {
        var n = 0;
        foreach (var kv in slots)
        {
            if (ItemWire602.IsBagSlot(kv.Key) && !ItemWire602.IsEmpty(kv.Value))
            {
                n++;
            }
        }

        return n;
    }

    static void MergeStackableBagItems(IDictionary<byte, byte[]> slots)
    {
        var bagKeys = slots.Keys.Where(ItemWire602.IsBagSlot).OrderBy(static x => x).ToList();
        foreach (var key in bagKeys)
        {
            if (!slots.TryGetValue(key, out var item) || ItemWire602.IsEmpty(item))
            {
                continue;
            }

            foreach (var otherKey in slots.Keys.Where(ItemWire602.IsBagSlot).OrderBy(static x => x).ToList())
            {
                if (otherKey <= key || !slots.TryGetValue(otherKey, out var other) || ItemWire602.IsEmpty(other))
                {
                    continue;
                }

                if (!ItemWire602.ItemsStackable(item, other))
                {
                    continue;
                }

                var merged = item.ToArray();
                var sum = Math.Min(255, (int)merged[2] + Math.Max(1, (int)other[2]));
                ItemWire602.SetDurability(merged, (byte)sum);
                slots[key] = merged;
                slots.Remove(otherKey);
                item = merged;
            }
        }
    }

    /// <summary>
    /// True when bag anchors overlap footprints, extend outside the 8×8 grid, or use invalid anchors.
    /// Used on join to decide whether full <see cref="CompactBagSlots"/> (repack) is needed.
    /// </summary>
    public static bool BagAnchorsHaveFootprintConflicts(IReadOnlyDictionary<byte, byte[]> slots)
    {
        var occupied = new bool[CellCount];
        foreach (var kv in slots)
        {
            if (!ItemWire602.IsBagSlot(kv.Key) || ItemWire602.IsEmpty(kv.Value))
            {
                continue;
            }

            if (!WireToCell(kv.Key, out var col, out var row))
            {
                return true;
            }

            ItemSizeCatalog.GetSize(kv.Value, out var w, out var h);
            if (w <= 0 || h <= 0 || col + w > Columns || row + h > Rows || col < 0 || row < 0)
            {
                return true;
            }

            for (var y = 0; y < h; y++)
            {
                for (var x = 0; x < w; x++)
                {
                    var idx = ((row + y) * Columns) + (col + x);
                    if ((uint)idx >= (uint)CellCount)
                    {
                        return true;
                    }

                    if (occupied[idx])
                    {
                        return true;
                    }

                    occupied[idx] = true;
                }
            }
        }

        return false;
    }

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
        var occupied = BuildOccupancy(slots, ignoreWireSlot);
        return IsRegionFree(occupied, col, row, w, h);
    }

    /// <summary>Repack bag items so anchors match client footprint (fixes overlapping DB/session slots).</summary>
    public static void RepackBagSlots(IDictionary<byte, byte[]> slots)
    {
        var wear = new List<KeyValuePair<byte, byte[]>>();
        var bag = new List<byte[]>();
        var toRemove = new List<byte>();
        foreach (var kv in slots)
        {
            if (ItemWire602.IsEmpty(kv.Value))
            {
                toRemove.Add(kv.Key);
                continue;
            }

            if (ItemWire602.IsWearSlot(kv.Key))
            {
                wear.Add(kv);
                continue;
            }

            if (ItemWire602.IsBagSlot(kv.Key))
            {
                bag.Add(kv.Value);
                toRemove.Add(kv.Key);
            }
        }

        foreach (var key in toRemove)
        {
            slots.Remove(key);
        }

        foreach (var kv in wear)
        {
            slots[kv.Key] = kv.Value;
        }

        bag.Sort(static (a, b) =>
        {
            ItemSizeCatalog.GetSize(b, out var wb, out var hb);
            ItemSizeCatalog.GetSize(a, out var wa, out var ha);
            var areaCmp = (wb * hb).CompareTo(wa * ha);
            return areaCmp != 0 ? areaCmp : ItemWire602.DecodeItemIndex(a).CompareTo(ItemWire602.DecodeItemIndex(b));
        });

        var repacked = new Dictionary<byte, byte[]>();
        var dropped = 0;
        foreach (var item12 in bag)
        {
            if (!TryFindEmptyAnchor(repacked, item12, out var anchor))
            {
                dropped++;
                continue;
            }

            repacked[anchor] = item12.ToArray();
        }

        if (dropped > 0)
        {
            Console.WriteLine("[m8] RepackBagSlots: dropped {0} bag item(s) (no footprint fit)", dropped);
        }

        foreach (var kv in repacked)
        {
            slots[kv.Key] = kv.Value;
        }
    }

    static bool[] BuildOccupancy(IReadOnlyDictionary<byte, byte[]> slots, byte? excludeSlot)
    {
        var occupied = new bool[CellCount];
        foreach (var kv in slots)
        {
            if (ItemWire602.IsEmpty(kv.Value) || !ItemWire602.IsBagSlot(kv.Key))
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
