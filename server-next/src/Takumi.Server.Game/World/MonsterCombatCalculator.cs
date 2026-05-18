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
        var baseDamage = fallbackDamage > 0
            ? fallbackDamage
            : Math.Max(1, (playerLevel * 8) + 10);
        var mitigated = baseDamage - Math.Max(0, target.Defense);
        mitigated = ApplyResistance(mitigated, attackElement, target);
        mitigated = ApplyElemental(mitigated, attackElement, target);
        if (damagePercent != 100)
        {
            mitigated = mitigated * damagePercent / 100;
        }

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
