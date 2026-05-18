using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>
/// Parity <c>ItemValue(ip,1)</c> sell zen: legacy buy-scale + socket bonus, then <c>/3</c>.
/// </summary>
public static class LegacyShopSellPriceEstimate
{
    public static long EstimateFromWire(ReadOnlySpan<byte> item12)
    {
        if (ItemWire602.IsEmpty(item12))
        {
            return 0;
        }

        var index = ItemWire602.DecodeItemIndex(item12);
        var level = ItemWire602.DecodeLevel(item12);
        var exc = item12[3] & 0x3F;
        var option = item12[1] & 0x07;
        var luck = (item12[1] & 0x04) != 0;
        var skill = (item12[1] & 0x80) != 0;

        var buyScale = LegacyShopBuyPriceEstimate.Estimate(index, level, exc, option, luck, skill);
        if (exc == 0
            && SocketItemTypeCatalog.IsSocketItem(index / 512, index % 512, out _)
            && CountActiveSockets(item12) > 0)
        {
            // Socket stock often has exc bits cleared on wire while client ItemValue still applies exc multipliers.
            var excStock = LegacyShopBuyPriceEstimate.Estimate(index, level, 63, option, luck, skill);
            if (excStock > buyScale)
            {
                buyScale = excStock;
            }
        }

        buyScale += SocketItemSellBonus.AddToBuyScale(item12, buyScale);
        return Math.Max(1, buyScale / 3);
    }

    static int CountActiveSockets(ReadOnlySpan<byte> item12)
    {
        if (item12.Length < ItemWire602.WireBytes)
        {
            return 0;
        }

        var count = 0;
        for (var i = 7; i < 12; i++)
        {
            var v = item12[i];
            if (v is not ItemWire602.NoSocket and not ItemWire602.EmptySocket)
            {
                count++;
            }
        }

        return count;
    }
}
