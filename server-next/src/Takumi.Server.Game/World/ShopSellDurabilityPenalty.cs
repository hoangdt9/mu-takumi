using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Parity <c>ItemValue(ip,1)</c> sell durability deduction (<c>repairGold = Gold * 0.6 * (1 - dur/max)</c>).</summary>
public static class ShopSellDurabilityPenalty
{
    public static long Apply(ReadOnlySpan<byte> item12, long sellZen)
    {
        if (sellZen <= 0 || item12.Length < ItemWire602.WireBytes)
        {
            return sellZen;
        }

        var index = ItemWire602.DecodeItemIndex(item12);
        if (index is < 0 or ItemWire602.ZenItemIndex)
        {
            return sellZen;
        }

        var group = index / 512;
        if (group == 14)
        {
            return sellZen;
        }

        var durability = item12[2];
        // Until BMD max-durability is wired, use wire durability vs 255 (parity ItemValue when max unknown).
        const int maxDurability = 255;
        if (durability >= maxDurability)
        {
            return sellZen;
        }

        var missingRatio = 1f - (durability / (float)maxDurability);
        var repairGold = (long)(sellZen * 0.6f * missingRatio);
        return Math.Max(0, sellZen - repairGold);
    }
}
