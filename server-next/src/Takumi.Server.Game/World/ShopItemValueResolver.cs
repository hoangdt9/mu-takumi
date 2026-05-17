using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Single source for shop buy/sell zen sent on <c>F3 E9</c> and charged on <c>0x32</c>.</summary>
public static class ShopItemValueResolver
{
    public static long ResolveBuy(NpcShopItemEntry item)
    {
        var index = (item.ItemGroup * 512) + item.ItemIndex;
        if (ItemValueCatalog.TryGetBuySellExact(index, item.ItemLevel, item.ExcOpt, out var buy, out _))
        {
            return buy;
        }

        // Wildcard OpExe (*) rows ignore excellent options; use legacy CItem::Value for exc shop stock.
        if (item.ExcOpt == 0
            && ItemValueCatalog.TryGetBuySell(index, item.ItemLevel, 0, out buy, out _))
        {
            return buy;
        }

        return LegacyShopBuyPriceEstimate.Estimate(
            index,
            item.ItemLevel,
            item.ExcOpt,
            item.Option,
            item.Luck != 0,
            item.Skill != 0);
    }

    public static long ResolveSell(ReadOnlySpan<byte> item12)
    {
        if (ItemWire602.IsEmpty(item12))
        {
            return 0;
        }

        var index = ItemWire602.DecodeItemIndex(item12);
        var level = (item12[1] >> 3) & 0x0F;
        var exc = item12[3] & 0x3F;
        if (ItemValueCatalog.TryGetBuySell(index, level, exc, out _, out var sell))
        {
            return sell;
        }

        var buy = LegacyShopBuyPriceEstimate.Estimate(
            index,
            level,
            exc,
            item12[1] & 0x07,
            (item12[1] & 0x04) != 0,
            (item12[1] & 0x80) != 0);
        return Math.Max(1, buy / 3);
    }

    public static ItemValueWire602.ItemValueEntry ToWireEntry(NpcShopItemEntry item)
    {
        var index = (item.ItemGroup * 512) + item.ItemIndex;
        if (ItemValueCatalog.TryGetWirePrice(index, item.ItemLevel, item.ExcOpt, out var priceType, out var value, out var sell)
            && priceType > 0)
        {
            return new(
                index,
                item.ItemLevel,
                item.ExcOpt,
                priceType,
                value,
                0,
                Math.Clamp(sell, 0, int.MaxValue));
        }

        var buy = (int)Math.Clamp(ResolveBuy(item), 0, int.MaxValue);
        sell = (int)Math.Clamp(Math.Max(1, buy / 3), 0, int.MaxValue);
        return new(index, item.ItemLevel, item.ExcOpt, 0, buy, 0, sell);
    }
}
