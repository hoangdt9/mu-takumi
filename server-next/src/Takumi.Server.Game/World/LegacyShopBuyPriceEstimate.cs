namespace Takumi.Server.Game.World;

/// <summary>Parity <c>CItem::Value</c> buy zen when <c>ItemValue.txt</c> has no row (not the env stub).</summary>
public static class LegacyShopBuyPriceEstimate
{
    public static long Estimate(int index, int itemLevel, int excOpt, int option, bool luck, bool skill)
    {
        ClientItemStatCatalog.EnsureInitialized();
        if (ClientItemStatCatalog.TryGetZen(index, out var zen) && zen > 0)
        {
            return zen;
        }

        var group = index / 512;
        var type = index % 512;
        var baseDrop = ClientItemStatCatalog.TryGetDropLevel(index, out var dl) ? dl : 0;

        if (group == 14 && ClientItemStatCatalog.TryGetValue(index, out var val) && val > 0)
        {
            return EstimatePotionValue(val, itemLevel, 255);
        }

        var itemLevel2 = baseDrop + itemLevel * 3;
        if (excOpt != 0 && group < 12)
        {
            itemLevel2 += 25;
        }

        itemLevel2 += itemLevel switch
        {
            5 => 4,
            6 => 10,
            7 => 25,
            8 => 45,
            9 => 65,
            10 => 95,
            11 => 135,
            12 => 185,
            13 => 245,
            14 => 305,
            15 => 365,
            _ => 0,
        };

        long price;
        if (IsWingLike(group, type))
        {
            price = ((itemLevel2 + 40L) * itemLevel2 * itemLevel2 * 11) + 100;
        }
        else if (group is 13 or 15)
        {
            price = (itemLevel2 * itemLevel2 * itemLevel2) + 100;
        }
        else
        {
            price = ((itemLevel2 + 40L) * itemLevel2 * itemLevel2 / 8) + 100;
            if (group is >= 0 and < 6 && ClientItemStatCatalog.TryGetTwoHand(index, out var twoHand) && !twoHand)
            {
                price = price * 80 / 100;
            }
        }

        if (skill)
        {
            price += price * 150 / 100;
        }

        if (luck)
        {
            price += price * 25 / 100;
        }

        if (option > 0)
        {
            price += option switch
            {
                1 => price * 60 / 100,
                2 => price * 140 / 100,
                3 => price * 280 / 100,
                4 => price * 560 / 100,
                _ => 0,
            };
        }

        if (group < 12 && excOpt != 0)
        {
            var excCount = CountExcOptions(excOpt);
            for (var i = 0; i < excCount; i++)
            {
                price += price;
            }
        }

        return Math.Clamp(price, 0, 2_000_000_000);
    }

    static bool IsWingLike(int group, int type) =>
        group == 12 && type is > 6 and < 36 or > 43 and not 50;

    static int CountExcOptions(int excOpt)
    {
        var n = 0;
        for (var b = 0; b < 6; b++)
        {
            if ((excOpt & (1 << b)) != 0)
            {
                n++;
            }
        }

        return n;
    }

    static long EstimatePotionValue(int val, int level, int durability)
    {
        long price = val * val * 10L / 12;
        if (level > 0)
        {
            price *= 1L << level;
        }

        price = price / 10 * 10;
        price *= durability;
        return price;
    }
}
