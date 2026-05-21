using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class SkillCombatRangeTests
{
    [Fact]
    public void Omnidirectional_center_is_caster_not_packet_aim()
    {
        var (cx, cy) = SkillCombatRange.GetAreaContinueCenter(46, 69, 41, 69, directional: false);
        Assert.Equal((46, 69), (cx, cy));
    }

    [Theory]
    [InlineData(41, 69, 35, 70, 6, true)]  // was edge-hit under old center; in square from player 46,69 too
    [InlineData(46, 69, 40, 64, 6, true)]
    [InlineData(46, 69, 29, 77, 6, false)]
    [InlineData(41, 69, 29, 77, 7, false)]
    public void Chebyshev_vs_manhattan_for_evil_spirit_tiles(
        byte centerX,
        byte centerY,
        byte mobX,
        byte mobY,
        int range,
        bool expectedChebyshev)
    {
        Assert.Equal(
            expectedChebyshev,
            SkillCombatRange.IsWithinChebyshev(centerX, centerY, mobX, mobY, range));
    }

    [Fact]
    public void Evil_spirit_uses_chebyshev_not_directional_arc()
    {
        const ushort evilSpirit = SkillCombatCatalog.EvilSpirit;
        Assert.False(SkillCombatCatalog.IsDirectionalContinueSkill(evilSpirit));
        Assert.Equal(7, SkillCombatCatalog.GetAreaContinueRange(evilSpirit));
        Assert.True(
            SkillCombatRange.IsMobInSkillVolume(
                evilSpirit,
                46,
                69,
                facingWire256: 175,
                40,
                64,
                range: 7));
        Assert.False(
            SkillCombatRange.IsMobInSkillVolume(
                evilSpirit,
                46,
                69,
                facingWire256: 175,
                29,
                77,
                range: 7));
    }

    [Fact]
    public void Twister_uses_forward_corridor_not_omnidirectional_square()
    {
        const ushort twister = SkillCombatCatalog.Twister;
        Assert.False(SkillCombatCatalog.IsDirectionalContinueSkill(twister));
        Assert.True(SkillCombatCatalog.IsForwardCorridorContinueSkill(twister));

        // Atlans log: caster ~(43,65), angle 34 — mob west (39,64) must not be hit.
        Assert.False(
            SkillCombatRange.IsMobInSkillVolume(twister, 43, 65, 34, 39, 64, range: 6));
        Assert.False(
            SkillCombatRange.IsMobInSkillVolume(twister, 43, 65, 34, 49, 65, range: 6));

        // Kanturu QA: player (221,38) angle 188 — mob far to the right must not be in corridor.
        Assert.False(
            SkillCombatRange.IsMobInSkillVolume(twister, 221, 38, 188, 229, 38, range: 6));
        Assert.False(
            SkillCombatRange.IsMobInSkillVolume(twister, 221, 38, 188, 221, 46, range: 6));
    }

    [Theory]
    [InlineData(55, true, false)]
    [InlineData(9, false, false)]
    [InlineData(8, false, true)]
    public void Catalog_continue_skill_modes(ushort skillId, bool directional, bool corridor)
    {
        Assert.Equal(directional, SkillCombatCatalog.IsDirectionalContinueSkill(skillId));
        Assert.Equal(corridor, SkillCombatCatalog.IsForwardCorridorContinueSkill(skillId));
    }
}
