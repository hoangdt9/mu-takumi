using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Stub zen prices until <c>ItemValue.txt</c> / <c>GCItemValueSend</c> parity.</summary>
public static class ShopItemPricing
{
    public static long BuyPrice(NpcShopItemEntry item) => ShopItemValueResolver.ResolveBuy(item);

    public static long SellPrice(ReadOnlySpan<byte> item12) => ShopItemValueResolver.ResolveSell(item12);

    public static long RepairCost(ReadOnlySpan<byte> item12, bool selfRepair = false)
    {
        if (ItemWire602.IsEmpty(item12))
        {
            return 0;
        }

        ItemValueCatalog.EnsureInitialized();
        ItemSizeCatalog.EnsureInitialized();
        var maxDur = ItemSizeCatalog.GetMaxDurability(item12);
        var cur = ItemWire602.DecodeDurability(item12);
        if (cur >= maxDur)
        {
            return 0;
        }

        var index = ItemWire602.DecodeItemIndex(item12);
        var level = ItemWire602.DecodeLevel(item12);
        var exc = ItemWire602.DecodeExcellentOptions(item12);
        long baseGold = 100;
        if (ItemValueCatalog.TryGetBuySellExact(index, level, exc, out var buy, out _))
        {
            baseGold = buy;
        }
        else if (ItemValueCatalog.TryGetBuySell(index, level, 0, out buy, out _))
        {
            baseGold = buy;
        }

        return ShopRepairPricing.Compute((int)Math.Min(baseGold, int.MaxValue), cur, maxDur, index, selfRepair);
    }

    static int ParseIntEnv(string name, int defaultValue, int min, int max)
    {
        var raw = Environment.GetEnvironmentVariable(name);
        if (!int.TryParse(raw, out var v))
        {
            v = defaultValue;
        }

        return Math.Clamp(v, min, max);
    }

    static long ParseLongEnv(string name, long defaultValue)
    {
        var raw = Environment.GetEnvironmentVariable(name);
        return long.TryParse(raw, out var v) ? v : defaultValue;
    }
}
