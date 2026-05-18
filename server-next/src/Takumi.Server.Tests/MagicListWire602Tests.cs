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
        Assert.Equal(28, pkt[5]);

        Assert.Contains(pkt, b => b == 236);
        Assert.Contains(pkt, b => b == 237);
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
}
