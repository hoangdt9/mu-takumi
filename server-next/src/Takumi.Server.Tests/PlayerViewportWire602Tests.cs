using System.Text;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class PlayerViewportWire602Tests
{
    [Fact]
    public void Viewport_wire_C2_12_single_entry_layout()
    {
        var name = new byte[10];
        Encoding.ASCII.GetBytes("Knight01").CopyTo(name);
        var pkt = PlayerViewportWire602.Build(
        [
            new PlayerViewportEntry(
                1001,
                120,
                130,
                120,
                130,
                0,
                name,
                1,
                0,
                true),
        ]);

        Assert.Equal(0xC2, pkt[0]);
        Assert.Equal(43, pkt.Length);
        Assert.Equal(0x12, pkt[3]);
        Assert.Equal(1, pkt[4]);
        Assert.Equal(0x83, pkt[5]);
        Assert.Equal(0xE9, pkt[6]);
        Assert.Equal(120, pkt[7]);
        Assert.Equal(130, pkt[8]);
        Assert.Equal(0, pkt[9]);
        Assert.Equal(0, pkt[42]);
    }
}
