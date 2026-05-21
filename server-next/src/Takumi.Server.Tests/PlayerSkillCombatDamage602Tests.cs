using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class PlayerSkillCombatDamage602Tests
{
    static CharacterSheetStats MgSwordBuild() =>
        CharacterSheetStats.FromInts(2000, 500, 500, 300, 0, 0);

    [Fact]
    public void FireSlash_uses_physical_roll_not_wizardry()
    {
        Assert.True(SkillCombatCatalog.UsesPhysicalStatRoll(55));
        Assert.False(SkillCombatCatalog.UsesWizardryStatRoll(55));
    }

    [Fact]
    public void Skill_55_hits_harder_than_melee_on_same_build()
    {
        var sheet = MgSwordBuild();
        const byte mgClass = 0x60;
        const ushort level = 400;

        Assert.True(PlayerSkillCombatDamage602.TryRollPhysiHit(
            mgClass, level, sheet, null, null, skillId: 0, Random.Shared, out var melee, out _));
        Assert.True(PlayerSkillCombatDamage602.TryRollPhysiHit(
            mgClass, level, sheet, null, null, skillId: 55, Random.Shared, out var slash, out _));

        var stat = new Game.World.MonsterStat(353, 85, 22000, 365, 395, 280, 3, 0, 2, 5, 10);
        var meleeFinal = Game.World.MonsterCombatCalculator.ApplySkillDamageToMonster(melee, 0, stat, level, skillId: 0);
        var slashFinal = Game.World.MonsterCombatCalculator.ApplySkillDamageToMonster(slash, 0, stat, level, skillId: 55);

        Assert.True(slashFinal > meleeFinal, $"slash={slashFinal} melee={meleeFinal} rawSlash={slash} rawMelee={melee}");
    }
}
