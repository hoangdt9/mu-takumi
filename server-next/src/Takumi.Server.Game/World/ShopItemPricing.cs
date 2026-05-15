using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Stub zen prices until <c>ItemValue.txt</c> / <c>GCItemValueSend</c> parity.</summary>
public static class ShopItemPricing
{
    public static long BuyPrice(NpcShopItemEntry item)
    {
        var index = (item.ItemGroup * 512) + item.ItemIndex;
        if (ItemValueCatalog.TryGetBuySell(index, item.ItemLevel, item.ExcOpt, out var buy, out _))
        {
            return buy;
        }

        var basePrice = ParseLongEnv("TAKUMI_SHOP_BUY_BASE", 1200);
        var perLevel = ParseLongEnv("TAKUMI_SHOP_BUY_PER_LEVEL", 400);
        return basePrice + (item.ItemLevel * perLevel) + (index % 17) * 10;
    }

    public static long SellPrice(ReadOnlySpan<byte> item12)
    {
        if (ItemWire602.IsEmpty(item12))
        {
            return 0;
        }

        var index = item12[0] | ((item12[3] & 0x80) << 1);
        var level = (item12[1] >> 3) & 0x0F;
        var exc = item12[3] & 0x3F;
        if (ItemValueCatalog.TryGetBuySell(index, level, exc, out _, out var sell))
        {
            return sell;
        }

        var buy = ParseLongEnv("TAKUMI_SHOP_BUY_BASE", 1200);
        var pct = ParseIntEnv("TAKUMI_SHOP_SELL_PCT", 33, 1, 100);
        var estimate = buy + level * 200;
        return Math.Max(1, estimate * pct / 100);
    }

    public static long RepairCost(ReadOnlySpan<byte> item12)
    {
        if (ItemWire602.IsEmpty(item12))
        {
            return 0;
        }

        var maxDur = 255;
        var cur = item12[2];
        if (cur >= maxDur)
        {
            return 0;
        }

        var perPoint = ParseLongEnv("TAKUMI_SHOP_REPAIR_PER_POINT", 2);
        return (maxDur - cur) * perPoint;
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
