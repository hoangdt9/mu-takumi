using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterCombatCalculatorTests
{
    [Fact]
    public void RollDamage_subtracts_defense_with_floor_one()
    {
        var stat = new MonsterStat(3, 5, 100, 10, 20, 15, 3, 0, 1, 5, 10);
        var dmg = MonsterCombatCalculator.RollDamageToMonster(playerLevel: 10, stat, fallbackDamage: 50);
        Assert.Equal(35, dmg);
    }

    [Fact]
    public void RollDamage_applies_skill_percent()
    {
        var stat = new MonsterStat(3, 5, 100, 10, 20, 15, 3, 0, 1, 5, 10);
        var dmg = MonsterCombatCalculator.RollDamageToMonster(playerLevel: 10, stat, fallbackDamage: 50, damagePercent: 150);
        Assert.Equal(52, dmg);
    }

    [Fact]
    public void RollDamage_never_below_one()
    {
        var stat = new MonsterStat(3, 1, 10, 0, 0, 999, 3, 0, 1, 5, 10);
        var dmg = MonsterCombatCalculator.RollDamageToMonster(playerLevel: 1, stat, fallbackDamage: 5);
        Assert.Equal(1, dmg);
    }
}
