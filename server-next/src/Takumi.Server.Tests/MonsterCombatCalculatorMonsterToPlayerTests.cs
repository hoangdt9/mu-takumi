using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterCombatCalculatorMonsterToPlayerTests
{
    [Fact]
    public void RollDamageFromMonsterToPlayer_uses_min_max_range()
    {
        var rng = new Random(42);
        var stat = new MonsterStat(
            Index: 1,
            Level: 5,
            Life: 100,
            DamageMin: 10,
            DamageMax: 20,
            Defense: 0,
            MoveRange: 3,
            AttackType: 0,
            AttackRange: 1,
            ViewRange: 5,
            RegenTimeSeconds: 10);
        for (var i = 0; i < 30; i++)
        {
            var d = MonsterCombatCalculator.RollDamageFromMonsterToPlayer(stat, playerDefense: 0, 100, rng, 99);
            Assert.InRange(d, 10, 20);
        }
    }

    [Fact]
    public void RollDamageFromMonsterToPlayer_subtracts_defense()
    {
        var rng = new Random(1);
        var stat = new MonsterStat(
            Index: 2,
            Level: 1,
            Life: 50,
            DamageMin: 30,
            DamageMax: 30,
            Defense: 0,
            MoveRange: 3,
            AttackType: 0,
            AttackRange: 1,
            ViewRange: 5,
            RegenTimeSeconds: 10);
        var d = MonsterCombatCalculator.RollDamageFromMonsterToPlayer(stat, playerDefense: 10, 100, rng, 5);
        Assert.Equal(20, d);
    }

    [Fact]
    public void RollDamageFromMonsterToPlayer_falls_back_when_txt_damage_zero()
    {
        var rng = new Random(7);
        var stat = new MonsterStat(
            Index: 3,
            Level: 1,
            Life: 10,
            DamageMin: 0,
            DamageMax: 0,
            Defense: 0,
            MoveRange: 3,
            AttackType: 0,
            AttackRange: 1,
            ViewRange: 5,
            RegenTimeSeconds: 10);
        var d = MonsterCombatCalculator.RollDamageFromMonsterToPlayer(stat, 0, 100, rng, fallbackDamageWhenTxtDamageZero: 44);
        Assert.Equal(44, d);
    }
}
