using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class SkillCombatCatalogMgTests
{
    [Theory]
    [InlineData(9)]
    [InlineData(8)]
    [InlineData(55)]
    [InlineData(56)]
    [InlineData(61)]
    [InlineData(236)]
    [InlineData(237)]
    [InlineData(238)]
    public void Mg_continue_skills_recognized(ushort skillId)
    {
        Assert.True(SkillCombatCatalog.IsAreaContinueSkill(skillId));
        Assert.True(SkillCombatCatalog.UsesMagicDamage(skillId));
        if (skillId is 55 or 56 or 236)
        {
            Assert.True(SkillCombatCatalog.UsesPhysicalStatRoll(skillId));
            Assert.False(SkillCombatCatalog.UsesWizardryStatRoll(skillId));
        }
        else if (skillId is 9 or 8)
        {
            Assert.True(SkillCombatCatalog.UsesWizardryStatRoll(skillId));
        }
        var expectedMinRange = skillId is 9 ? 7 : skillId is 8 ? 6 : 2;
        if (skillId is 9)
        {
            Assert.Equal(7, SkillCombatCatalog.GetAreaContinueRange(skillId));
        }
        else if (skillId is 8)
        {
            Assert.Equal(6, SkillCombatCatalog.GetAreaContinueRange(skillId));
        }
        Assert.True(SkillCombatCatalog.GetAreaContinueRange(skillId) >= expectedMinRange);
    }

    [Theory]
    [InlineData(55)]
    [InlineData(236)]
    public void Mg_directional_slash_short_range(ushort skillId)
    {
        Assert.True(SkillCombatCatalog.IsDirectionalContinueSkill(skillId));
        Assert.Equal(2, SkillCombatCatalog.GetAreaContinueRange(skillId));
    }

    [Fact]
    public void Gigantic_storm_omnidirectional_chebyshev_radius()
    {
        Assert.Equal(6, SkillCombatCatalog.GetAreaContinueRange(SkillCombatCatalog.GiganticStorm));
        Assert.False(SkillCombatCatalog.IsDirectionalContinueSkill(SkillCombatCatalog.GiganticStorm));
        Assert.False(SkillCombatCatalog.IsForwardCorridorContinueSkill(SkillCombatCatalog.GiganticStorm));
        Assert.Equal(110, SkillCombatCatalog.GetSkillBaseDamage(SkillCombatCatalog.GiganticStorm));
        Assert.Equal(160, SkillCombatCatalog.GetSkillFinalMultiplierPercent(SkillCombatCatalog.GiganticStorm));
        Assert.True(SkillCombatCatalog.UsesWizardryStatRoll(SkillCombatCatalog.GiganticStorm));
    }

    [Theory]
    [InlineData(3)]
    [InlineData(4)]
    [InlineData(5)]
    public void Targeted_magic_skills(ushort skillId)
    {
        Assert.True(SkillCombatCatalog.IsTargetedMagicSkill(skillId));
        Assert.False(SkillCombatCatalog.IsAreaContinueSkill(skillId));
        Assert.True(SkillCombatCatalog.GetTargetedSkillRange(skillId) >= 6);
    }

    [Fact]
    public void Mg_class_index()
    {
        Assert.True(SkillCombatCatalog.IsMagicGladiator(0x60)); // MG wire class 96 / 0x60 typical
    }

    [Fact]
    public void Mg_magic_preview_nonzero()
    {
        var sheet = CharacterSheetStats.FromInts(100, 100, 100, 500, 0, 0);
        var preview = CharacterCombatPreview602.FromSheet(0x60, 100, sheet);
        Assert.True(preview.MagicDamageMax > 0);
    }
}
