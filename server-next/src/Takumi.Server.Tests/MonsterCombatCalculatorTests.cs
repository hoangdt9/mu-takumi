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
        Assert.Equal(75, dmg);
    }

    [Fact]
    public void RollDamage_applies_skill_percent()
    {
        var stat = new MonsterStat(3, 5, 100, 10, 20, 15, 3, 0, 1, 5, 10);
        var dmg = MonsterCombatCalculator.RollDamageToMonster(playerLevel: 10, stat, fallbackDamage: 50, damagePercent: 150);
        Assert.Equal(112, dmg);
    }

    [Fact]
    public void RollDamage_never_below_one()
    {
        var stat = new MonsterStat(3, 1, 10, 0, 0, 999, 3, 0, 1, 5, 10);
        var dmg = MonsterCombatCalculator.RollDamageToMonster(playerLevel: 1, stat, fallbackDamage: 5);
        Assert.Equal(1, dmg);
    }

    [Fact]
    public void RollDamage_uses_level_not_stub_floor_on_high_defense_mobs()
    {
        var stat = new MonsterStat(352, 82, 18000, 335, 365, 335, 3, 0, 2, 5, 10);
        var dmg = MonsterCombatCalculator.RollDamageToMonster(playerLevel: 400, stat, fallbackDamage: 50);
        Assert.Equal(2875, dmg);
    }

    [Fact]
    public void ApplyResistance_reduces_damage_by_percent()
    {
        var stat = new MonsterStat(1, 1, 10, 0, 0, 0, 3, 0, 1, 5, 10, Resistance0: 50);
        var dmg = MonsterCombatCalculator.ApplyResistance(100, attackElement: 0, stat);
        Assert.Equal(50, dmg);
    }

    [Fact]
    public void ApplyElemental_subtracts_elemental_defense_when_matched()
    {
        var stat = new MonsterStat(1, 1, 10, 0, 0, 0, 3, 0, 1, 5, 10, ElementalAttribute: 2, ElementalDefense: 20);
        var dmg = MonsterCombatCalculator.ApplyElemental(100, attackElement: 2, stat);
        Assert.Equal(80, dmg);
    }

    [Fact]
    public void ApplyClientDamageTypeMultiplier_excellent_and_double()
    {
        var dmg = MonsterCombatCalculator.ApplyClientDamageTypeMultiplier(1000, damageType: 0x42);
        Assert.Equal(2400, dmg);
    }

    [Fact]
    public void ApplyClientDamageTypeMultiplier_critical()
    {
        var dmg = MonsterCombatCalculator.ApplyClientDamageTypeMultiplier(1000, damageType: 0x03);
        Assert.Equal(1300, dmg);
    }

    [Fact]
    public void ApplySkillDamage_uses_level_floor_when_defense_eats_roll()
    {
        var stat = new MonsterStat(353, 85, 22000, 365, 395, 280, 3, 0, 2, 5, 10);
        var dmg = MonsterCombatCalculator.ApplySkillDamageToMonster(rolledDamage: 300, attackElement: 0, stat, playerLevel: 400, skillId: 55);
        Assert.Equal(40, dmg);
    }

    [Fact]
    public void ApplySkillDamage_still_subtracts_defense_when_above_floor()
    {
        var stat = new MonsterStat(353, 85, 22000, 365, 395, 280, 3, 0, 2, 5, 10);
        var dmg = MonsterCombatCalculator.ApplySkillDamageToMonster(rolledDamage: 1200, attackElement: 0, stat, playerLevel: 100);
        Assert.True(dmg >= 900);
    }
}
