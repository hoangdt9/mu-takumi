namespace Takumi.Server.Game.World;

/// <summary>Repair zen cost (parity client <c>ConvertRepairGold</c> in <c>ZzzInventory.cpp</c>).</summary>
public static class ShopRepairPricing
{
    public static long Compute(int baseGold, int currentDurability, int maxDurability, int itemIndex, bool selfRepair)
    {
        if (maxDurability <= 0 || currentDurability >= maxDurability)
        {
            return 0;
        }

        var gold = Math.Min(baseGold, 400_000_000);
        var repairGold = (float)gold;
        var percent = 1f - (currentDurability / (float)maxDurability);
        if (percent <= 0f)
        {
            return 0;
        }

        var fRoot = MathF.Sqrt(repairGold);
        var fRootRoot = MathF.Sqrt(MathF.Sqrt(repairGold));
        repairGold = 3f * fRoot * fRootRoot;
        repairGold *= percent;
        repairGold++;

        if (currentDurability <= 0)
        {
            var group = itemIndex / 512;
            var indexInGroup = itemIndex % 512;
            // ITEM_HELPER+4 / +5 (Dark Spirit): double cost at 0 durability.
            if (group == 14 && indexInGroup is 4 or 5)
            {
                repairGold *= 2f;
            }
            else
            {
                repairGold += repairGold * 0.4f;
            }
        }

        var cost = (int)repairGold;
        if (selfRepair)
        {
            cost += (int)(repairGold * 1.5f);
        }

        return Math.Max(0, cost);
    }
}
