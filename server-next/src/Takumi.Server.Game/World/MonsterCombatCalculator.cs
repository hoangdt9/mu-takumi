using System.Globalization;

namespace Takumi.Server.Game.World;

/// <summary>Minimal player→monster damage (parity <c>CAttack::Attack</c> simplified).</summary>
public static class MonsterCombatCalculator
{
    public static int RollDamageToMonster(
        int playerLevel,
        MonsterStat target,
        int fallbackDamage,
        int damagePercent = 100,
        int attackElement = 0)
    {
        // Level-based attack must not lose to TAKUMI_COMBAT_STUB_DAMAGE (default 50): that value was
        // clamping every hit on high-defense mobs (e.g. Kanturu Iron Rider) to 1 damage.
        var levelBased = Math.Max(1, (playerLevel * 8) + 10);
        var baseDamage = fallbackDamage > 0
            ? Math.Max(fallbackDamage, levelBased)
            : levelBased;
        var mitigated = baseDamage - Math.Max(0, target.Defense);
        mitigated = ApplyResistance(mitigated, attackElement, target);
        mitigated = ApplyElemental(mitigated, attackElement, target);
        if (damagePercent != 100)
        {
            mitigated = mitigated * damagePercent / 100;
        }

        return Math.Clamp(mitigated, 1, 65_000);
    }

    /// <summary>Apply monster defense/resist to a pre-rolled wizardry hit.</summary>
    public static int ApplySkillDamageToMonster(int rolledDamage, int attackElement, MonsterStat target)
    {
        if (rolledDamage <= 0)
        {
            return 0;
        }

        var mitigated = rolledDamage - Math.Max(0, target.Defense);
        mitigated = ApplyResistance(mitigated, attackElement, target);
        mitigated = ApplyElemental(mitigated, attackElement, target);
        return Math.Clamp(mitigated, 1, 65_000);
    }

    /// <summary>Resistance slot 0–3 from <c>Monster.txt</c> (physical / poison / ice / fire stub).</summary>
    public static int ApplyResistance(int damage, int attackElement, MonsterStat target)
    {
        if (damage <= 0)
        {
            return damage;
        }

        var resistPct = attackElement switch
        {
            1 => target.Resistance1,
            2 => target.Resistance2,
            3 => target.Resistance3,
            _ => target.Resistance0,
        };
        var factor = 100 - Math.Clamp(resistPct, 0, 90);
        return Math.Max(1, damage * factor / 100);
    }

    /// <summary>Season 7+ elemental columns when present in <c>Monster.txt</c>.</summary>
    public static int ApplyElemental(int damage, int attackElement, MonsterStat target)
    {
        if (damage <= 0 || target.ElementalAttribute <= 0 || attackElement <= 0)
        {
            return damage;
        }

        var mismatch = target.ElementalAttribute != 6 && target.ElementalAttribute != attackElement;
        if (mismatch)
        {
            return damage;
        }

        var mitigated = damage - Math.Max(0, target.ElementalDefense);
        return Math.Max(1, mitigated);
    }

    public static int ResolveAttackElement()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_COMBAT_ATTACK_ELEMENT");
        if (string.IsNullOrWhiteSpace(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var v))
        {
            return 0;
        }

        return Math.Clamp(v, 0, 6);
    }

    public static bool RollMiss(int missRatePercent)
    {
        if (missRatePercent <= 0)
        {
            return false;
        }

        return Random.Shared.Next(100) < Math.Clamp(missRatePercent, 0, 100);
    }

    /// <summary>
    /// Client <c>ReceiveAttackDamage</c> damage-type nibble (0 normal, 1 perfect, 2 excellent, 3 critical)
    /// plus optional double (bit 6) and combo (bit 7).
    /// </summary>
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

    /// <summary>
    /// Apply legacy-style bonuses for the rolled client damage type (excellent 120%, critical max-ish, double/combo bits).
    /// Call after base mitigation; <paramref name="damageType"/> is the wire byte (nibble + 0x40/0x80).
    /// </summary>
    public static int ApplyClientDamageTypeMultiplier(int damage, byte damageType)
    {
        if (damage <= 0)
        {
            return damage;
        }

        var result = damage;
        switch (damageType & 0x0F)
        {
            case 1: // perfect
                result = result * 110 / 100;
                break;
            case 2: // excellent — legacy ~120% of max roll
                result = result * 120 / 100;
                break;
            case 3: // critical — legacy uses max + bonus; approximate with +30%
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

    /// <summary>Physical hit from monster toward player (minimal parity with legacy <c>gAttack</c> damage roll).</summary>
    public static int RollDamageFromMonsterToPlayer(
        MonsterStat attacker,
        int playerDefense,
        int damagePercent,
        Random rng,
        int fallbackDamageWhenTxtDamageZero)
    {
        var min = attacker.DamageMin;
        var max = attacker.DamageMax;
        if (max < min)
        {
            (min, max) = (max, min);
        }

        int rolled;
        if (min <= 0 && max <= 0)
        {
            rolled = Math.Max(1, fallbackDamageWhenTxtDamageZero);
        }
        else
        {
            rolled = min == max ? min : rng.Next(min, max + 1);
            rolled = Math.Max(1, rolled);
        }

        rolled = rolled * Math.Clamp(damagePercent, 50, 500) / 100;
        var mitigated = rolled - Math.Max(0, playerDefense);
        return Math.Clamp(mitigated, 1, 65_000);
    }

    /// <summary>Stub defense until roster stores armor (legacy derives from stats + equipment).</summary>
    public static int ResolveStubPlayerDefense(int playerLevel)
    {
        var perLevel = ParseIntEnv("TAKUMI_MONSTER_TO_PLAYER_DEF_PER_LEVEL", 3, 0, 50);
        var flat = ParseIntEnv("TAKUMI_COMBAT_PLAYER_DEFENSE_FLAT", 0, 0, 10_000);
        return flat + Math.Max(0, playerLevel) * perLevel;
    }

    /// <summary>Player→player hit (parity <c>CAttack</c> PvP branch simplified).</summary>
    public static int RollDamagePlayerToPlayer(
        int attackerLevel,
        int victimLevel,
        int damagePercent = 100,
        int fallbackDamage = 0)
    {
        var baseDamage = fallbackDamage > 0
            ? fallbackDamage
            : Math.Max(1, attackerLevel * 6 + 8);
        if (damagePercent != 100)
        {
            baseDamage = baseDamage * damagePercent / 100;
        }

        var defense = ResolveStubPlayerDefense(victimLevel);
        return Math.Clamp(baseDamage - defense, 1, 65_000);
    }

    static int ParseIntEnv(string name, int defaultValue, int min, int max)
    {
        var raw = Environment.GetEnvironmentVariable(name);
        if (!int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var v))
        {
            v = defaultValue;
        }

        return Math.Clamp(v, min, max);
    }
}
