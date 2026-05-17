namespace Takumi.Server.Game.World;

/// <summary>Parity <c>ItemValue(ip,1)</c> buy zen when <c>ItemValue.txt</c> has no row (not the env stub).</summary>
public static class LegacyShopBuyPriceEstimate
{
    /// <summary>Shop file <c>Option</c> column → wire Option3 (<c>ItemConvert</c>).</summary>
    public static int DeriveWireOption3(int shopOption) => (shopOption & 3) + (shopOption > 3 ? 4 : 0);

    public static long Estimate(int index, int itemLevel, int excOpt, int option, bool luck, bool skill)
    {
        var option3 = DeriveWireOption3(option);
        var group = index / 512;
        var core = EstimateCore(index, itemLevel, includeExcLevelBonus: false, option3, luck, skill);
        if (excOpt == 0 || group >= 12)
        {
            return core;
        }

        var excCount = CountExcOptions(excOpt);
        if (excCount <= 0)
        {
            return core;
        }

        // Client ItemValue on converted shop items: net excellent multiplier ≈ 2×popcount(exc)
        // on the skill/luck core (elf-lala +9 exc63 bow ≈ 3.34M × 12 ≈ 40M).
        return Math.Clamp(core * (2L * excCount), 0, 2_000_000_000);
    }

    static long EstimateCore(
        int index,
        int itemLevel,
        bool includeExcLevelBonus,
        int option3,
        bool luck,
        bool skill)
    {
        ClientItemStatCatalog.EnsureInitialized();
        if (ClientItemStatCatalog.TryGetZen(index, out var zen) && zen > 0)
        {
            return zen;
        }

        var group = index / 512;
        var type = index % 512;
        var baseDrop = ClientItemStatCatalog.TryGetDropLevel(index, out var dl) ? dl : 0;

        if (group == 14)
        {
            return EstimatePotionBuy(index, itemLevel);
        }

        var itemLevel2 = baseDrop + itemLevel * 3;
        if (includeExcLevelBonus && group < 12)
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

        if (option3 > 0)
        {
            price += option3 switch
            {
                4 => price * 60 / 100,
                8 => price * 140 / 100,
                12 => price * 280 / 100,
                16 => price * 560 / 100,
                _ => 0,
            };
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

    /// <summary>Parity <c>ItemValue(ip,1)</c> for consumables (full shop stack durability 255).</summary>
    static long EstimatePotionBuy(int index, int level)
    {
        const int stackDurability = 255;
        var itemType = index % 512;
        long price;
        if (itemType is 3 or 6)
        {
            // Client ITEM_POTION+3/+6 uses flat 1500 before durability multiply.
            price = 1500;
        }
        else if (ClientItemStatCatalog.TryGetValue(index, out var val) && val > 0)
        {
            price = val * val * 10L / 12;
        }
        else
        {
            return 0;
        }

        if (level > 0)
        {
            price *= 1L << level;
        }

        price = price / 10 * 10;
        price *= stackDurability;
        price /= 3;
        return price / 10 * 10;
    }
}
