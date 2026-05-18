using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Verbose shop buy/sell tracing. Enable with <c>TAKUMI_SHOP_PRICE_TRACE=1</c>.</summary>
public static class ShopPriceTrace
{
    static readonly bool Enabled = IsTruthy(Environment.GetEnvironmentVariable("TAKUMI_SHOP_PRICE_TRACE"));

    public static bool IsEnabled => Enabled;

    public static void Line(string tag, string message) =>
        Write(tag, message);

    public static void SellTransaction(
        string remote,
        byte invSlot,
        long zenBefore,
        long credited,
        long zenAfter,
        ReadOnlySpan<byte> wire)
    {
        if (!Enabled)
        {
            return;
        }

        Write(
            "sell-0x33",
            $"remote={remote} invSlot={invSlot} zenBefore={zenBefore} credited={credited} zenAfter={zenAfter} delta={zenAfter - zenBefore} wire={WireHex(wire)}");
        LogWireDecoded("sell-0x33", wire);
        LogSellBreakdown("sell-0x33", wire);
    }

    public static void BuyTransaction(
        string remote,
        byte shopSlot,
        long zenBefore,
        long charged,
        long zenAfter,
        NpcShopItemEntry item,
        int taxPercent)
    {
        if (!Enabled)
        {
            return;
        }

        var index = (item.ItemGroup * 512) + item.ItemIndex;
        Write(
            "buy-0x32",
            $"remote={remote} shopSlot={shopSlot} zenBefore={zenBefore} charged={charged} zenAfter={zenAfter} " +
            $"index={index} g={item.ItemGroup} i={item.ItemIndex} lvl={item.ItemLevel} exc={item.ExcOpt} " +
            $"opt={item.Option} luck={item.Luck} skill={item.Skill} tax%={taxPercent} " +
            $"resolveBuy={ShopItemValueResolver.ResolveBuy(item)} chargedBuy={ShopItemValueResolver.ResolveChargedBuy(item, taxPercent)}");
    }

    public static void F3E9Row(NpcShopItemEntry item, int taxPercent, ReadOnlySpan<byte> wire, ItemValueWire602.ItemValueEntry entry)
    {
        if (!Enabled)
        {
            return;
        }

        Write(
            "F3-E9",
            $"index={entry.Index} lvl={entry.Level} exc={entry.NewOpt} buy={entry.Value} sell={entry.SellValue} " +
            $"priceType={entry.Type} catalogExc={item.ExcOpt} wire={WireHex(wire)}");
    }

    public static void LogSellBreakdown(string tag, ReadOnlySpan<byte> wire)
    {
        if (!Enabled || ItemWire602.IsEmpty(wire))
        {
            return;
        }

        var index = ItemWire602.DecodeItemIndex(wire);
        var level = ItemWire602.DecodeLevel(wire);
        var exc = wire[3] & 0x3F;
        var option = wire[1] & 0x07;
        var luck = (wire[1] & 0x04) != 0;
        var skill = (wire[1] & 0x80) != 0;
        var durability = wire[2];
        var legacyPath = ShopItemValueResolver.ShouldUseLegacySellFromWireForTrace(wire);
        var isSocket = SocketItemTypeCatalog.IsSocketItem(index / 512, index % 512, out var maxSock);

        long preDurSell;
        string path;
        if (legacyPath)
        {
            preDurSell = LegacyShopSellPriceEstimate.EstimateFromWire(wire);
            path = "legacy-wire";
        }
        else
        {
            preDurSell = ShopItemValueResolver.ResolveSellFromPartsForTrace(
                index, level, exc, option, luck, skill, out path);
        }

        var finalSell = ShopItemValueResolver.ResolveSell(wire);
        var socketCount = CountActiveSockets(wire);

        Write(
            tag,
            $"path={path} legacy={legacyPath} socketItem={isSocket} maxSocket={maxSock} activeSockets={socketCount} " +
            $"preDurSell={preDurSell} finalSell={finalSell} durPenalty={preDurSell - finalSell} " +
            $"exactRow={ItemValueCatalog.TryGetBuySellExact(index, level, exc, out var eb, out var es)} " +
            $"buyExact={eb} sellExact={es} " +
            $"wildcardRow={ItemValueCatalog.TryGetBuySell(index, level, exc, out var wb, out var ws)} " +
            $"buyWild={wb} sellWild={ws} " +
            $"legacyBuy={LegacyShopBuyPriceEstimate.Estimate(index, level, exc, option, luck, skill)} " +
            $"legacyBuyExc63={LegacyShopBuyPriceEstimate.Estimate(index, level, 63, option, luck, skill)}");
    }

    public static void LogWireDecoded(string tag, ReadOnlySpan<byte> wire)
    {
        if (!Enabled || wire.Length < ItemWire602.WireBytes)
        {
            return;
        }

        var index = ItemWire602.DecodeItemIndex(wire);
        Write(
            tag,
            $"decoded index={index} g={index / 512} i={index % 512} lvl={(wire[1] >> 3) & 15} " +
            $"opt={wire[1] & 7} luck={((wire[1] & 4) != 0)} skill={((wire[1] & 0x80) != 0)} " +
            $"dur={wire[2]} exc={wire[3] & 0x3F} b4={wire[4]} b5={wire[5]} " +
            $"sock=[{wire[7]},{wire[8]},{wire[9]},{wire[10]},{wire[11]}]");
    }

    static int CountActiveSockets(ReadOnlySpan<byte> wire)
    {
        var n = 0;
        for (var i = 7; i < 12; i++)
        {
            if (wire[i] is not ItemWire602.NoSocket and not ItemWire602.EmptySocket)
            {
                n++;
            }
        }

        return n;
    }

    static void Write(string tag, string message) =>
        Console.WriteLine("[shop-price] {0} {1}", tag, message);

    static string WireHex(ReadOnlySpan<byte> wire) =>
        wire.Length >= ItemWire602.WireBytes
            ? Convert.ToHexString(wire[..ItemWire602.WireBytes])
            : string.Empty;

    static bool IsTruthy(string? raw) =>
        !string.IsNullOrWhiteSpace(raw)
        && raw is not "0"
        && !string.Equals(raw.Trim(), "false", StringComparison.OrdinalIgnoreCase);
}
