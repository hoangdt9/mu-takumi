using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterWalkWire602Tests
{
    [Fact]
    public void BuildWithFacing_includes_path_nibble_for_client()
    {
        var pkt = MonsterWalkWire602.BuildWithFacing(0, 133, 168, angle1To8: 3);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(8, pkt[1]);
        Assert.Equal(0xD4, pkt[2]);
        Assert.Equal(133, pkt[5]);
        Assert.Equal(168, pkt[6]);
        Assert.Equal(0x20, pkt[7]); // (3-1)<<4
    }
}
