using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterViewportWire602Tests
{
    [Fact]
    public void Build_single_monster_C2_13_layout()
    {
        var pkt = MonsterViewportWire602.Build(
        [
            new MonsterViewportEntry(12001, 3, 100, 110, 100, 110, 3, CreateFlag: true),
        ]);

        Assert.Equal(0xC2, pkt[0]);
        Assert.Equal(0x13, pkt[3]);
        Assert.Equal(1, pkt[4]);
        Assert.Equal(15, pkt.Length);
        Assert.Equal(0xAE, pkt[5]);
        Assert.Equal(0xE1, pkt[6]);
        Assert.Equal(0, pkt[7]);
        Assert.Equal(3, pkt[8]);
        Assert.Equal(100, pkt[9]);
        Assert.Equal(110, pkt[10]);
        Assert.Equal(100, pkt[11]);
        Assert.Equal(110, pkt[12]);
        Assert.Equal(0x30, pkt[13]);
        Assert.Equal(0, pkt[14]);
    }
}
