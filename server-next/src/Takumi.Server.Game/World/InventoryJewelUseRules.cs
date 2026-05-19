using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Jewel-on-item use (parity <c>CObjectManager::CharacterUseJewelOfBles</c>).</summary>
public static class InventoryJewelUseRules
{
    public const int ItemJewelOfBless = (14 * 512) + 13;

    public const int ItemFenrir = (13 * 512) + 37;

    /// <summary>Restore a broken Fenrir (durability &lt; 255) using one Jewel of Bless.</summary>
    public static bool CanRepairFenrirWithBless(int sourceItemIndex, ReadOnlySpan<byte> targetItem12)
    {
        if (sourceItemIndex != ItemJewelOfBless)
        {
            return false;
        }

        if (ItemWire602.DecodeItemIndex(targetItem12) != ItemFenrir)
        {
            return false;
        }

        return ItemWire602.DecodeDurability(targetItem12) < 255;
    }
}
