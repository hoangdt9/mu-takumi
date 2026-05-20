using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MagicListWire602Tests
{
    [Fact]
    public void BuildMagicGladiatorFull_IncludesDuelMasterSkills()
    {
        var pkt = MagicListWire602.BuildMagicGladiatorFull(20);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(0xF3, pkt[2]);
        Assert.Equal(0x11, pkt[3]);
        Assert.Equal(44, pkt[4]);
        Assert.Equal(MagicListWire602.ListTypeNormal, pkt[5]);

        var types = ParseSkillTypes(pkt);
        Assert.Contains(types, t => t == 236);
        Assert.Contains(types, t => t == 237);
        Assert.Contains(types, t => t == 61);
        Assert.Contains(types, t => t == 238);
        Assert.Contains(types, t => t == 490);
    }

    [Fact]
    public void BuildForServerClass_MgRosterClass_SendsSkills()
    {
        var pkt = MagicListWire602.BuildForServerClass(120);
        Assert.True(pkt.Length > 7);
    }

    [Fact]
    public void BuildForServerClass_DwRosterClass_SendsStarterSkills()
    {
        var pkt = MagicListWire602.BuildForServerClass(0);
        Assert.True(pkt.Length > 7);
        Assert.Contains(pkt, b => b == 4);
    }

    [Fact]
    public void BuildAddSkill_uses_FE_value_and_skill_bytes()
    {
        var pkt = MagicListWire602.BuildAddSkill(55, 55, 1);
        Assert.Equal(10, pkt[1]);
        Assert.Equal(0xFE, pkt[4]);
        Assert.Equal(MagicListWire602.ListTypeNormal, pkt[5]);
        Assert.Equal(55, pkt[6]);
        Assert.Equal(55, pkt[7]);
        Assert.Equal(0, pkt[8]);
        Assert.Equal(1, pkt[9]);
    }

    static ushort[] ParseSkillTypes(byte[] pkt)
    {
        var count = pkt[4];
        var types = new ushort[count];
        for (var i = 0; i < count; i++)
        {
            var o = 6 + (i * 4);
            types[i] = (ushort)(pkt[o + 1] | (pkt[o + 2] << 8));
        }

        return types;
    }
}
