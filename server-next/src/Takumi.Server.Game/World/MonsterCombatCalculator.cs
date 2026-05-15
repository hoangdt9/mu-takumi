namespace Takumi.Server.Game.World;

/// <summary>Minimal player→monster damage (parity <c>CAttack::Attack</c> simplified).</summary>
public static class MonsterCombatCalculator
{
    public static int RollDamageToMonster(
        int playerLevel,
        MonsterStat target,
        int fallbackDamage,
        int damagePercent = 100)
    {
        var baseDamage = fallbackDamage > 0
            ? fallbackDamage
            : Math.Max(1, (playerLevel * 8) + 10);
        var mitigated = baseDamage - Math.Max(0, target.Defense);
        if (damagePercent != 100)
        {
            mitigated = mitigated * damagePercent / 100;
        }

        return Math.Clamp(mitigated, 1, 65_000);
    }

    public static bool RollMiss(int missRatePercent)
    {
        if (missRatePercent <= 0)
        {
            return false;
        }

        return Random.Shared.Next(100) < Math.Clamp(missRatePercent, 0, 100);
    }
}
