using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Parity <c>CSocketItemMgr::CalcSocketBonusItemValue</c> (buy-scale, before <c>/3</c> sell).</summary>
public static class SocketItemSellBonus
{
    const int MaxSocketOption = 50;
    const int MaxSocketTypes = 6;

    public static long AddToBuyScale(ReadOnlySpan<byte> item12, long buyScale)
    {
        if (item12.Length < ItemWire602.WireBytes || buyScale <= 0)
        {
            return 0;
        }

        var index = ItemWire602.DecodeItemIndex(item12);
        if (!SocketItemTypeCatalog.IsSocketItem(index / 512, index % 512, out _))
        {
            return 0;
        }

        var socketCount = 0;
        long bonus = 0;
        for (var i = 7; i < 12; i++)
        {
            var v = item12[i];
            if (v is ItemWire602.NoSocket or ItemWire602.EmptySocket)
            {
                continue;
            }

            socketCount++;
            bonus += EstimateSeedSphereBuyZen(v);
        }

        if (socketCount <= 0)
        {
            return 0;
        }

        bonus += (long)(buyScale * (socketCount * 0.8));
        return bonus;
    }

    static long EstimateSeedSphereBuyZen(byte socketByte)
    {
        var seedId = socketByte % MaxSocketOption;
        var sphereLevel = (socketByte / MaxSocketOption) + 1;
        if (seedId is 0xFF or 0xFE)
        {
            return 0;
        }

        int seedType;
        if (seedId <= 9)
        {
            seedType = 0;
        }
        else if (seedId <= 15)
        {
            seedType = 1;
        }
        else if (seedId <= 20)
        {
            seedType = 2;
        }
        else if (seedId <= 28)
        {
            seedType = 3;
        }
        else if (seedId <= 33)
        {
            seedType = 4;
        }
        else if (seedId <= 40)
        {
            seedType = 5;
        }
        else
        {
            return 0;
        }

        var wingIndex = (12 * 512) + 100 + ((sphereLevel - 1) * MaxSocketTypes) + seedType;
        return LegacyShopBuyPriceEstimate.Estimate(wingIndex, 0, 0, 0, false, false);
    }
}
