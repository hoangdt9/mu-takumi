namespace Takumi.Server.Protocol;

/// <summary>Per-item damage/defense after level, excellent, and durability (parity <c>CItem::Convert</c> core).</summary>
public static class ItemInstanceCombatCalculator
{
    public readonly record struct ItemCombatStats(
        int DamageMin,
        int DamageMax,
        int Defense,
        int DefenseSuccessRate,
        int MagicDamageRate,
        int AttackSpeed,
        float DurabilityState);

    public static ItemCombatStats Compute(ReadOnlySpan<byte> item12)
    {
        if (ItemWire602.IsEmpty(item12))
        {
            return default;
        }

        var index = ItemWire602.DecodeItemIndex(item12);
        if (index < 0 || !ItemCombatStatCatalog.TryGet(index, out var baseStats))
        {
            return default;
        }

        var level = ItemWire602.DecodeLevel(item12);
        var durability = ItemWire602.DecodeDurability(item12);
        var newOption = ItemWireDecode602.DecodeNewOption(item12);
        var setOption = ItemWireDecode602.DecodeSetOption(item12);

        var itemLevel = Math.Max(0, (int)baseStats.DropLevel);
        if (newOption != 0 || setOption != 0)
        {
            itemLevel += 25;
        }

        if (setOption != 0)
        {
            itemLevel = Math.Max(itemLevel, baseStats.DropLevel + 30);
        }

        var damageMin = (int)baseStats.DamageMin;
        var damageMax = (int)baseStats.DamageMax;
        var defense = (int)baseStats.Defense;
        var defRate = (int)baseStats.DefenseSuccessRate;
        var magicRate = (int)baseStats.MagicPower;

        ApplyDamageScaling(ref damageMin, ref damageMax, baseStats.DropLevel, itemLevel, level, newOption, setOption, index);
        ApplyMagicRateScaling(ref magicRate, baseStats.DropLevel, itemLevel, level, newOption, setOption, index);
        ApplyDefenseScaling(ref defense, ref defRate, baseStats.DropLevel, itemLevel, level, newOption, setOption, index);

        var durabilityState = ComputeDurabilityState(damageMin, damageMax, durability);
        if (durability == 0)
        {
            return default;
        }

        damageMin = (int)(damageMin * durabilityState);
        damageMax = (int)(damageMax * durabilityState);
        defense = (int)(defense * durabilityState);
        defRate = (int)(defRate * durabilityState);

        return new ItemCombatStats(
            Math.Max(0, damageMin),
            Math.Max(0, damageMax),
            Math.Max(0, defense),
            Math.Max(0, defRate),
            Math.Max(0, magicRate),
            baseStats.AttackSpeed,
            durabilityState);
    }

    static void ApplyDamageScaling(
        ref int damageMin,
        ref int damageMax,
        int dropLevel,
        int itemLevel,
        int level,
        int newOption,
        int setOption,
        int index)
    {
        if (damageMax <= 0 && damageMin <= 0)
        {
            return;
        }

        if (setOption != 0 && dropLevel != 0)
        {
            var bonus = ((damageMin * 25) / dropLevel) + 5;
            damageMin += bonus + (itemLevel / 40) + 5;
            damageMax += bonus + (itemLevel / 40) + 5;
        }
        else if (newOption != 0 && dropLevel != 0)
        {
            var chaos = ChaosWeaponBonus(index);
            var bonus = chaos != 0 ? chaos : ((damageMin * 25) / dropLevel) + 5;
            damageMin += bonus;
            damageMax += bonus;
        }

        damageMin += level * 3;
        damageMax += level * 3;

        if (level >= 10)
        {
            var extra = ((level - 9) * (level - 8)) / 2;
            damageMin += extra;
            damageMax += extra;
        }
    }

    static void ApplyMagicRateScaling(
        ref int magicRate,
        int dropLevel,
        int itemLevel,
        int level,
        int newOption,
        int setOption,
        int index)
    {
        if (magicRate <= 0)
        {
            return;
        }

        if (setOption != 0 && dropLevel != 0)
        {
            magicRate += ((magicRate * 25) / dropLevel) + 5;
            magicRate += (itemLevel / 60) + 2;
        }
        else if (newOption != 0 && dropLevel != 0)
        {
            var chaos = ChaosWeaponBonus(index);
            magicRate += chaos != 0 ? chaos : ((magicRate * 25) / dropLevel) + 5;
        }

        magicRate += level * 3;
        if (level >= 10)
        {
            magicRate += ((level - 9) * (level - 8)) / 2;
        }
    }

    static void ApplyDefenseScaling(
        ref int defense,
        ref int defRate,
        int dropLevel,
        int itemLevel,
        int level,
        int newOption,
        int setOption,
        int index)
    {
        if (ItemWireDecode602.IsShieldIndex(index))
        {
            if (defense > 0)
            {
                defense += level;
                if (setOption != 0 && itemLevel != 0)
                {
                    defense += ((defense * 20) / itemLevel) + 2;
                }
            }

            return;
        }

        if (defense > 0)
        {
            if (setOption != 0 && dropLevel != 0 && itemLevel != 0)
            {
                defense += (((defense * 12) / dropLevel) + (dropLevel / 5)) + 4;
                defense += (((defense * 3) / itemLevel) + (itemLevel / 30)) + 2;
            }
            else if (newOption != 0 && dropLevel != 0)
            {
                defense += (((defense * 12) / dropLevel) + (dropLevel / 5)) + 4;
            }

            if (ItemWireDecode602.IsWingIndex(index))
            {
                defense += level * 4;
            }
            else
            {
                defense += level * 3;
            }

            if (level >= 10)
            {
                defense += ((level - 9) * (level - 8)) / 2;
            }
        }

        if (defRate > 0)
        {
            if (setOption != 0 && dropLevel != 0)
            {
                defRate += ((defRate * 25) / dropLevel) + 5;
                defRate += (itemLevel / 40) + 5;
            }
            else if (newOption != 0 && dropLevel != 0)
            {
                defRate += ((defRate * 25) / dropLevel) + 5;
            }

            defRate += level * 3;
            if (level >= 10)
            {
                defRate += ((level - 9) * (level - 8)) / 2;
            }
        }
    }

    static int ChaosWeaponBonus(int index) =>
        index switch
        {
            (2 * 512) + 6 => 15,
            (5 * 512) + 7 => 25,
            (4 * 512) + 6 => 30,
            _ => 0,
        };

    static float ComputeDurabilityState(int damageMin, int damageMax, byte durability)
    {
        if (durability == 0)
        {
            return 0f;
        }

        var baseDur = Math.Max(1, Math.Max(damageMin, damageMax) + 20);
        var s3 = baseDur * 0.3f;
        var s2 = baseDur * 0.4f;
        var s1 = baseDur * 0.5f;
        if (durability < s3)
        {
            return 0.5f;
        }

        if (durability < s2)
        {
            return 0.6f;
        }

        if (durability < s1)
        {
            return 0.7f;
        }

        return 1f;
    }
}
