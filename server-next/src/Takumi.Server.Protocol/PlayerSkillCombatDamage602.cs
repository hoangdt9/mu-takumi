namespace Takumi.Server.Protocol;

/// <summary>Roll skill hit damage from roster stats + equipment (parity client <c>GetMagicSkillDamage</c>).</summary>
public static class PlayerSkillCombatDamage602
{
    public static bool TryRollWizardryHit(
        byte serverClass,
        ushort level,
        CharacterSheetStats sheet,
        IReadOnlyDictionary<byte, byte[]>? wearSlots,
        CombatEffectState602? activeEffects,
        ushort skillId,
        Random rng,
        out int damage,
        out byte damageType)
    {
        damage = 0;
        damageType = 0;

        if (!SkillCombatCatalog.UsesWizardryStatRoll(skillId))
        {
            return false;
        }

        return TryRollSkillHit(
            serverClass,
            level,
            sheet,
            wearSlots,
            activeEffects,
            skillId,
            useMagic: true,
            rng,
            out damage,
            out damageType);
    }

    public static bool TryRollPhysiHit(
        byte serverClass,
        ushort level,
        CharacterSheetStats sheet,
        IReadOnlyDictionary<byte, byte[]>? wearSlots,
        CombatEffectState602? activeEffects,
        ushort skillId,
        Random rng,
        out int damage,
        out byte damageType)
    {
        damage = 0;
        damageType = 0;

        if (!SkillCombatCatalog.UsesPhysicalStatRoll(skillId) && skillId != 0)
        {
            return false;
        }

        return TryRollSkillHit(
            serverClass,
            level,
            sheet,
            wearSlots,
            activeEffects,
            skillId,
            useMagic: false,
            rng,
            out damage,
            out damageType);
    }

    static bool TryRollSkillHit(
        byte serverClass,
        ushort level,
        CharacterSheetStats sheet,
        IReadOnlyDictionary<byte, byte[]>? wearSlots,
        CombatEffectState602? activeEffects,
        ushort skillId,
        bool useMagic,
        Random rng,
        out int damage,
        out byte damageType)
    {
        damage = 0;
        damageType = 0;

        var skillBase = SkillCombatCatalog.GetSkillBaseDamage(skillId);
        var calc = CharacterCombatCalculator602.Compute(
            serverClass,
            level,
            sheet,
            wearSlots,
            activeEffects);
        var preview = calc.Combat;

        int min;
        int max;
        if (useMagic)
        {
            // Webzen GetMagicSkillDamage: MagicMin/Max + SkillAttribute.Damage (+Damage/2 on max).
            min = (int)preview.MagicDamageMin + skillBase;
            max = (int)preview.MagicDamageMax + skillBase + (skillBase / 2);
            ApplyDamageMultipliers(ref min, ref max, calc.MulMagicDamage, calc.MagicDamageRate);
        }
        else
        {
            // Webzen GetAttackDamage: PhysiMin/Max + skill component (Power Slash may have skillBase=0).
            min = (int)preview.PhysiDamageMin + skillBase;
            max = (int)preview.PhysiDamageMax + skillBase + (skillBase / 2);
            ApplyDamageMultipliers(ref min, ref max, calc.MulPhysiDamage, 0);
        }

        if (max < min)
        {
            max = min;
        }

        if (min <= 0 && max <= 0)
        {
            min = Math.Max(1, level * 4 + skillBase);
            max = min + Math.Max(skillBase / 2, 10);
        }

        damage = min >= max ? min : rng.Next(min, max + 1);
        damageType = MonsterCombatDamageRoll602.RollClientDamageType(rng, isSkill: skillId > 0);
        damage = MonsterCombatDamageRoll602.ApplyClientDamageTypeMultiplier(damage, damageType);
        return damage > 0;
    }

    static void ApplyDamageMultipliers(ref int min, ref int max, uint mulPercent, uint ratePercent)
    {
        if (mulPercent > 0)
        {
            min += min * (int)mulPercent / 100;
            max += max * (int)mulPercent / 100;
        }

        if (ratePercent > 0)
        {
            min += min * (int)ratePercent / 100;
            max += max * (int)ratePercent / 100;
        }
    }
}

/// <summary>Shared damage roll helpers (extracted for skill + monster combat).</summary>
public static class MonsterCombatDamageRoll602
{
    public static byte RollClientDamageType(Random rng, bool isSkill)
    {
        var excellentPct = ParseIntEnv("TAKUMI_COMBAT_EXCELLENT_PCT", isSkill ? 12 : 18, 0, 80);
        var criticalPct = ParseIntEnv("TAKUMI_COMBAT_CRITICAL_PCT", isSkill ? 8 : 12, 0, 80);
        var perfectPct = ParseIntEnv("TAKUMI_COMBAT_PERFECT_PCT", 4, 0, 50);
        var doublePct = ParseIntEnv("TAKUMI_COMBAT_DOUBLE_PCT", isSkill ? 6 : 10, 0, 50);
        var comboPct = ParseIntEnv("TAKUMI_COMBAT_COMBO_PCT", 3, 0, 30);

        var roll = rng.Next(100);
        byte typeNibble = 0;
        if (roll < criticalPct)
        {
            typeNibble = 3;
        }
        else if (roll < criticalPct + excellentPct)
        {
            typeNibble = 2;
        }
        else if (roll < criticalPct + excellentPct + perfectPct)
        {
            typeNibble = 1;
        }

        var damageType = typeNibble;
        if (rng.Next(100) < doublePct)
        {
            damageType |= 0x40;
        }

        if (rng.Next(100) < comboPct)
        {
            damageType |= 0x80;
        }

        return damageType;
    }

    public static int ApplyClientDamageTypeMultiplier(int damage, byte damageType)
    {
        if (damage <= 0)
        {
            return damage;
        }

        var result = damage;
        switch (damageType & 0x0F)
        {
            case 1:
                result = result * 110 / 100;
                break;
            case 2:
                result = result * 120 / 100;
                break;
            case 3:
                result = result * 130 / 100;
                break;
        }

        if ((damageType & 0x40) != 0)
        {
            result *= 2;
        }

        if ((damageType & 0x80) != 0)
        {
            result += result / 2;
        }

        return Math.Clamp(result, 1, 65_000);
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
}
