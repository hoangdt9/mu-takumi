using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class SkillCombatDirectionTests
{
    [Theory]
    [InlineData(0, 0, 0, 2, 0, true)]   // east, mob east
    [InlineData(0, 0, 0, -2, 0, false)] // mob west (behind)
    [InlineData(0, 0, 64, 0, 2, true)]   // ~90°, mob north
    [InlineData(0, 0, 0, 0, 3, false)]  // out of range 2
    public void Forward_arc_filters_behind_and_range(
        byte ox,
        byte oy,
        byte angle,
        int dx,
        int dy,
        bool expected)
    {
        var actual = SkillCombatDirection.IsInForwardArc(
            ox,
            oy,
            angle,
            (byte)(ox + dx),
            (byte)(oy + dy),
            maxRange: 2);
        Assert.Equal(expected, actual);
    }

    [Theory]
    [InlineData(0, 0, 0, 2, 0, true)]   // east, mob 2 tiles ahead on line
    [InlineData(0, 0, 0, 0, 3, false)]  // 3 tiles lateral — outside corridor width
    [InlineData(0, 0, 0, -2, 0, false)] // behind caster
    public void Forward_corridor_filters_lateral_and_behind(
        byte ox,
        byte oy,
        byte angle,
        int dx,
        int dy,
        bool expected)
    {
        var actual = SkillCombatDirection.IsInForwardCorridor(
            ox,
            oy,
            angle,
            (byte)(ox + dx),
            (byte)(oy + dy),
            maxForwardTiles: 6);
        Assert.Equal(expected, actual);
    }

    [Theory]
    [InlineData(55, true, false, 2)]
    [InlineData(236, true, false, 2)]
    [InlineData(9, false, false, 6)]
    [InlineData(8, false, true, 6)]
    public void Catalog_range_and_mode_flags(ushort skillId, bool directional, bool corridor, int range)
    {
        Assert.Equal(directional, SkillCombatCatalog.IsDirectionalContinueSkill(skillId));
        Assert.Equal(corridor, SkillCombatCatalog.IsForwardCorridorContinueSkill(skillId));
        Assert.Equal(range, SkillCombatCatalog.GetAreaContinueRange(skillId));
    }
}
