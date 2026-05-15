using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterCombatCalculatorTests
{
    [Fact]
    public void RollDamage_subtracts_defense_with_floor_one()
    {
        var stat = new MonsterStat(3, Level: 5, Life: 100, DamageMin: 10, DamageMax: 20, Defense: 15, RegenTimeSeconds: 10);
        var dmg = MonsterCombatCalculator.RollDamageToMonster(playerLevel: 10, stat, fallbackDamage: 50);
        Assert.Equal(35, dmg);
    }

    [Fact]
    public void RollDamage_never_below_one()
    {
        var stat = new MonsterStat(3, Level: 1, Life: 10, DamageMin: 0, DamageMax: 0, Defense: 999, RegenTimeSeconds: 10);
        var dmg = MonsterCombatCalculator.RollDamageToMonster(playerLevel: 1, stat, fallbackDamage: 5);
        Assert.Equal(1, dmg);
    }
}
