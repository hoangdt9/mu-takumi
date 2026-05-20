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
        Assert.Equal((byte)3, learn.RequiredClassIndex);
    }

    [Fact]
    public void Mg_class_can_learn_fire_slash()
    {
        const int fireSlashOrb = (12 * 512) + 16;
        InventorySkillOrbRules.TryGetSkillOrbLearn(fireSlashOrb, out var learn);
        Assert.True(InventorySkillOrbRules.CanCharacterLearn(0x78, learn));
    }
}
