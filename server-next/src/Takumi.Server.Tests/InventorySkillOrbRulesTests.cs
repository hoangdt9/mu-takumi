using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class InventorySkillOrbRulesTests
{
    [Fact]
    public void FireSlash_orb_maps_to_skill_55()
    {
        const int fireSlashOrb = (12 * 512) + 16;
        Assert.True(InventorySkillOrbRules.TryGetSkillOrbLearn(fireSlashOrb, out var learn));
        Assert.Equal((ushort)55, learn.SkillType);
        Assert.Equal((byte)60, learn.MinLevel);
        Assert.Equal(1u << 3, learn.AllowedClassMask); // MG only
    }

    [Fact]
    public void Mg_class_can_learn_fire_slash()
    {
        const int fireSlashOrb = (12 * 512) + 16;
        InventorySkillOrbRules.TryGetSkillOrbLearn(fireSlashOrb, out var learn);
        Assert.True(InventorySkillOrbRules.CanCharacterLearn(0x78, learn));
    }

    [Fact]
    public void FireBurst_scroll_maps_and_DL_can_learn()
    {
        const int scroll = (12 * 512) + 21;
        Assert.True(InventorySkillOrbRules.TryGetSkillOrbLearn(scroll, out var learn));
        Assert.Equal((ushort)61, learn.SkillType);
        Assert.Equal((byte)74, learn.MinLevel);
        // DL sample class byte: class index 4 (0x20 * 4 = 0x80)
        Assert.True(InventorySkillOrbRules.CanCharacterLearn(0x90, learn));
        Assert.False(InventorySkillOrbRules.CanCharacterLearn(0x78, learn)); // MG
    }

    [Fact]
    public void Cometfall_etc_scroll_7692_maps_skill_13_with_energy_gate()
    {
        const int idx = (15 * 512) + 12;
        Assert.True(InventoryEtcSkillScrollRules.TryGetSkillScrollLearn(idx, out var learn));
        Assert.Equal((ushort)13, learn.SkillType);
        Assert.Equal((byte)80, learn.MinLevel);
        Assert.Equal((ushort)436, learn.MinEnergy);
        Assert.True((learn.AllowedClassMask & (1u << 0)) != 0); // DW
        Assert.True((learn.AllowedClassMask & (1u << 3)) != 0); // MG
        Assert.True(InventorySkillOrbRules.CanCharacterLearn(0x78, learn));
    }
}
