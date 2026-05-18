using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Minimal equip slot checks (parity <c>CheckItemMoveToInventory</c> simplified).</summary>
public static class InventoryEquipRules
{
    public static bool CanMoveBetweenSlots(byte sourceSlot, byte targetSlot, ReadOnlySpan<byte> sourceItem)
    {
        if (ItemWire602.IsEmpty(sourceItem))
        {
            return false;
        }

        if (ItemWire602.IsBagSlot(sourceSlot) && ItemWire602.IsBagSlot(targetSlot))
        {
            return true;
        }

        if (ItemWire602.IsWearSlot(sourceSlot) && ItemWire602.IsWearSlot(targetSlot))
        {
            return CanEquipToWearSlot(targetSlot, sourceItem);
        }

        if (ItemWire602.IsBagSlot(sourceSlot) && ItemWire602.IsWearSlot(targetSlot))
        {
            return CanEquipToWearSlot(targetSlot, sourceItem);
        }

        if (ItemWire602.IsWearSlot(sourceSlot) && ItemWire602.IsBagSlot(targetSlot))
        {
            return true;
        }

        return false;
    }

    public static bool CanEquipToWearSlot(byte wearSlot, ReadOnlySpan<byte> item12)
    {
        if (!ItemWire602.IsWearSlot(wearSlot) || ItemWire602.IsZenItem(item12))
        {
            return false;
        }

        var index = ItemWire602.DecodeItemIndex(item12);
        if (index < 0)
        {
            return false;
        }

        var group = index / 512;
        if (group is >= 0 and <= 5)
        {
            return wearSlot <= 1;
        }

        if (group == 6)
        {
            return wearSlot == 1;
        }

        if (group == 7)
        {
            return wearSlot == 2;
        }

        if (group == 8)
        {
            return wearSlot == 3;
        }

        if (group == 9)
        {
            return wearSlot == 4;
        }

        if (group == 10)
        {
            return wearSlot == 5;
        }

        if (group == 11)
        {
            return wearSlot == 6;
        }

        if (group == 12)
        {
            return wearSlot is 7 or 8;
        }

        // Helper / pet / muun (ITEM_HELPER group 14, custom pets group 13) → slot 8; rings/amulet → 9–11.
        if (group is 13 or 14)
        {
            return wearSlot is 8 or 9 or 10 or 11;
        }

        return false;
    }
}
