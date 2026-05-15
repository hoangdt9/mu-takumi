using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class PlayerPresenceWire602Tests
{
    [Fact]
    public void Position_wire_C1_15_layout()
    {
        var pkt = PlayerPositionWire602.Build(1001, 120, 130);
        Assert.Equal(new byte[] { 0xC1, 0x07, 0x15, 0x03, 0xE9, 120, 130 }, pkt);
    }

    [Fact]
    public void Action_wire_C1_18_layout()
    {
        var pkt = PlayerActionWire602.Build(1001, dir: 4, action: 2, targetKey: 12001);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(9, pkt[1]);
        Assert.Equal(0x18, pkt[2]);
        Assert.Equal(0x03, pkt[3]);
        Assert.Equal(0xE9, pkt[4]);
        Assert.Equal(4, pkt[5]);
        Assert.Equal(2, pkt[6]);
        Assert.Equal(0x2E, pkt[7]);
        Assert.Equal(0xE1, pkt[8]);
    }
}
