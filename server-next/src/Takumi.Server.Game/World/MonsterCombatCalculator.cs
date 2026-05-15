namespace Takumi.Server.Game.World;

/// <summary>Minimal player→monster damage (parity <c>CAttack::Attack</c> simplified).</summary>
public static class MonsterCombatCalculator
{
    public static int RollDamageToMonster(int playerLevel, MonsterStat target, int fallbackDamage)
    {
        var baseDamage = fallbackDamage > 0
            ? fallbackDamage
            : Math.Max(1, (playerLevel * 8) + 10);
        var mitigated = baseDamage - Math.Max(0, target.Defense);
        return Math.Clamp(mitigated, 1, 65_000);
    }
}
