using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class SkillCombatDirectionTests
{
    [Theory]
    [InlineData(50, 50, 128, 52, 50, true)]   // wire 128 → east; mob east
    [InlineData(50, 50, 128, 48, 50, false)] // mob west (behind)
    [InlineData(50, 50, 192, 50, 52, true)]  // wire 192 → north; mob north
    [InlineData(50, 50, 128, 50, 53, false)] // out of arc range 2
    public void Forward_arc_filters_behind_and_range(
        byte ox,
        byte oy,
        byte angle,
        byte tx,
        byte ty,
        bool expected)
    {
        var actual = SkillCombatDirection.IsInForwardArc(
            ox,
            oy,
            angle,
            tx,
            ty,
            maxRange: 2);
        Assert.Equal(expected, actual);
    }

    [Theory]
    [InlineData(50, 50, 0, 48, 50, true)]   // wire 0 + 180° → west; mob west on line
    [InlineData(50, 50, 0, 52, 50, false)]  // east = behind
    [InlineData(50, 50, 0, 50, 53, false)]  // lateral — outside corridor width
    public void Forward_corridor_filters_lateral_and_behind(
        byte ox,
        byte oy,
        byte angle,
        byte tx,
        byte ty,
        bool expected)
    {
        var actual = SkillCombatDirection.IsInForwardCorridor(
            ox,
            oy,
            angle,
            tx,
            ty,
            maxForwardTiles: 6);
        Assert.Equal(expected, actual);
    }

    [Theory]
    [InlineData(55, true, false, 2)]
    [InlineData(236, true, false, 2)]
    [InlineData(237, false, false, 6)]
    [InlineData(9, false, false, 7)]
    [InlineData(8, false, true, 6)]
    public void Catalog_range_and_mode_flags(ushort skillId, bool directional, bool corridor, int range)
    {
        Assert.Equal(directional, SkillCombatCatalog.IsDirectionalContinueSkill(skillId));
        Assert.Equal(corridor, SkillCombatCatalog.IsForwardCorridorContinueSkill(skillId));
        Assert.Equal(range, SkillCombatCatalog.GetAreaContinueRange(skillId));
    }
}
