using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterWalkWire602Tests
{
    [Fact]
    public void Build_monster_walk_C1_D4_layout()
    {
        var pkt = MonsterWalkWire602.Build(12001, 100, 110);

        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(7, pkt[1]);
        Assert.Equal(0xD4, pkt[2]);
        Assert.Equal(0x2E, pkt[3]);
        Assert.Equal(0xE1, pkt[4]);
        Assert.Equal(100, pkt[5]);
        Assert.Equal(110, pkt[6]);
    }
}
