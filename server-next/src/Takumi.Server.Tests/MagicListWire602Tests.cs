using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MagicListWire602Tests
{
    [Fact]
    public void BuildMagicGladiatorCombatQa_has_thirty_compact_combat_skills()
    {
        var pkt = MagicListWire602.BuildMagicGladiatorCombatQa(20);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(0xF3, pkt[2]);
        Assert.Equal(0x11, pkt[3]);
        Assert.Equal(30, pkt[4]);
        Assert.Equal(MagicListWire602.ListTypeNormal, pkt[5]);

        var parsed = ParseSkills(pkt);
        Assert.Equal(30, parsed.Count);
        Assert.Equal((byte)1, parsed[0].Index);
        Assert.Equal((ushort)1, parsed[0].Type);
        Assert.Equal((byte)30, parsed[29].Index);
        Assert.Equal((ushort)237, parsed[29].Type);

        Assert.Contains(parsed, s => s.Type == 55);
        Assert.Contains(parsed, s => s.Type == 236);
        Assert.Contains(parsed, s => s.Type == 76);
        Assert.DoesNotContain(parsed, s => s.Type == 61);
        Assert.DoesNotContain(parsed, s => s.Type == 238);
        Assert.DoesNotContain(parsed, s => s.Type == 490);
    }

    [Fact]
    public void BuildMagicGladiatorFull_matches_combat_qa_alias()
    {
        var qa = MagicListWire602.BuildMagicGladiatorCombatQa(20);
        var full = MagicListWire602.BuildMagicGladiatorFull(20);
        Assert.Equal(qa, full);
    }

    [Fact]
    public void BuildForServerClass_MgRosterClass_SendsSkills()
    {
        var pkt = MagicListWire602.BuildForServerClass(120);
        Assert.True(pkt.Length > 7);
        Assert.Equal(30, pkt[4]);
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
        var pkt = MagicListWire602.BuildAddSkill(24, 55, 1);
        Assert.Equal(10, pkt[1]);
        Assert.Equal(0xFE, pkt[4]);
        Assert.Equal(MagicListWire602.ListTypeNormal, pkt[5]);
        Assert.Equal(24, pkt[6]);
        Assert.Equal(55, pkt[7]);
        Assert.Equal(0, pkt[8]);
        Assert.Equal(1, pkt[9]);
    }

    static List<(byte Index, ushort Type, byte Level)> ParseSkills(byte[] pkt)
    {
        var count = pkt[4];
        var list = new List<(byte, ushort, byte)>(count);
        for (var i = 0; i < count; i++)
        {
            var o = 6 + (i * 4);
            list.Add((pkt[o], (ushort)(pkt[o + 1] | (pkt[o + 2] << 8)), pkt[o + 3]));
        }

        return list;
    }
}
