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

    /// <summary>Buy zen charged on <c>0x32</c> (base + NPC shop tax). Same value as F3 E9 wire <c>Value</c>.</summary>
    public static long ResolveChargedBuy(NpcShopItemEntry item, int taxRatePercent)
    {
        var buy = ResolveBuy(item);
        if (taxRatePercent <= 0)
        {
            return buy;
        }

        return buy + (buy * taxRatePercent / 100);
    }

    public static long ResolveSell(ReadOnlySpan<byte> item12)
    {
        if (ItemWire602.IsEmpty(item12))
        {
            return 0;
        }

        long sell;
        if (ShouldUseLegacySellFromWire(item12))
        {
            sell = LegacyShopSellPriceEstimate.EstimateFromWire(item12);
        }
        else
        {
            var index = ItemWire602.DecodeItemIndex(item12);
            var level = (item12[1] >> 3) & 0x0F;
            var exc = item12[3] & 0x3F;
            sell = ResolveSellFromParts(
                index,
                level,
                exc,
                item12[1] & 0x07,
                (item12[1] & 0x04) != 0,
                (item12[1] & 0x80) != 0);
        }

        return ShopSellDurabilityPenalty.Apply(item12, sell);
    }

    internal static bool ShouldUseLegacySellFromWireForTrace(ReadOnlySpan<byte> item12) =>
        ShouldUseLegacySellFromWire(item12);

    internal static long ResolveSellFromPartsForTrace(
        int index,
        int level,
        int exc,
        int option,
        bool luck,
        bool skill,
        out string path)
    {
        if (ItemValueCatalog.TryGetBuySellExact(index, level, exc, out var buyExact, out var sellExact))
        {
            path = sellExact > 0 ? "itemvalue-exact-sell" : "itemvalue-exact-buy/3";
            return sellExact > 0 ? sellExact : Math.Max(1, buyExact / 3);
        }

        if (exc == 0
            && ItemValueCatalog.TryGetBuySell(index, level, 0, out var buyPlain, out var sellPlain))
        {
            path = sellPlain > 0 ? "itemvalue-wildcard-sell" : "itemvalue-wildcard-buy/3";
            return sellPlain > 0 ? sellPlain : Math.Max(1, buyPlain / 3);
        }

        path = "legacy-buy/3";
        var buy = LegacyShopBuyPriceEstimate.Estimate(index, level, exc, option, luck, skill);
        return Math.Max(1, buy / 3);
    }

    static bool ShouldUseLegacySellFromWire(ReadOnlySpan<byte> item12)
    {
        var index = ItemWire602.DecodeItemIndex(item12);
        var exc = item12[3] & 0x3F;
        if (exc != 0)
        {
            return true;
        }

        return SocketItemTypeCatalog.IsSocketItem(index / 512, index % 512, out _);
    }

    /// <summary>
    /// Sell-back zen must follow the same <c>ItemValue.txt</c> / legacy rules as <see cref="ResolveBuy"/>.
    /// Wildcard OpExe rows (exc=*) must not be used for excellent stock — they ignore exc multipliers and
    /// made F3 E9 tooltips show ~13M sell while <c>0x32</c> credited ~4M.
    /// </summary>
    static long ResolveSellFromParts(
        int index,
        int level,
        int exc,
        int option,
        bool luck,
        bool skill)
    {
        if (ItemValueCatalog.TryGetBuySellExact(index, level, exc, out var buyExact, out var sellExact))
        {
            return sellExact > 0 ? sellExact : Math.Max(1, buyExact / 3);
        }

        if (exc == 0
            && ItemValueCatalog.TryGetBuySell(index, level, 0, out var buyPlain, out var sellPlain))
        {
            return sellPlain > 0 ? sellPlain : Math.Max(1, buyPlain / 3);
        }

        var buy = LegacyShopBuyPriceEstimate.Estimate(index, level, exc, option, luck, skill);
        return Math.Max(1, buy / 3);
    }

    public static ItemValueWire602.ItemValueEntry ToWireEntry(
        NpcShopItemEntry item,
        int taxRatePercent,
        ReadOnlySpan<byte> itemWire12 = default)
    {
        var index = (item.ItemGroup * 512) + item.ItemIndex;
        if (itemWire12.Length >= ItemWire602.WireBytes)
        {
            index = ItemWire602.DecodeItemIndex(itemWire12);
        }

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

        var chargedBuy = (int)Math.Clamp(ResolveChargedBuy(item, taxRatePercent), 0, int.MaxValue);
        // Tooltip sell must match 0x32 sell credit (ResolveSell on the same wire as inserted into bag).
        long sellZen;
        if (itemWire12.Length >= ItemWire602.WireBytes)
        {
            sellZen = ResolveSell(itemWire12);
        }
        else
        {
            sellZen = Math.Max(1, chargedBuy / 3);
        }

        var sellValue = (int)Math.Clamp(sellZen, 0, int.MaxValue);
        return new(index, item.ItemLevel, item.ExcOpt, 0, chargedBuy, 0, sellValue);
    }
}
