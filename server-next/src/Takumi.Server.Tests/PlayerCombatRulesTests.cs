using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class PlayerCombatRulesTests
{
    [Fact]
    public void RollDamagePlayerToPlayer_respects_victim_defense()
    {
        var vsLow = MonsterCombatCalculator.RollDamagePlayerToPlayer(50, 1, 100, fallbackDamage: 200);
        var vsHigh = MonsterCombatCalculator.RollDamagePlayerToPlayer(50, 80, 100, fallbackDamage: 200);
        Assert.True(vsLow > vsHigh);
        Assert.True(vsHigh >= 1);
    }

    [Fact]
    public void CanAttackPlayer_blocks_when_out_of_melee_range()
    {
        try
        {
            Environment.SetEnvironmentVariable("TAKUMI_COMBAT_PVP_ENABLED", "1");
            Environment.SetEnvironmentVariable("TAKUMI_COMBAT_PVP_MELEE_RANGE", "3");
            Assert.False(PlayerCombatRules.CanAttackPlayer(0, 10, 10, 20, 20));
            Assert.True(PlayerCombatRules.CanAttackPlayer(0, 10, 10, 12, 11));
        }
        finally
        {
            Environment.SetEnvironmentVariable("TAKUMI_COMBAT_PVP_ENABLED", null);
            Environment.SetEnvironmentVariable("TAKUMI_COMBAT_PVP_MELEE_RANGE", null);
        }
    }
}
