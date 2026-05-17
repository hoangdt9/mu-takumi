using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class PersonalShopWire602Tests
{
    [Fact]
    public void BuildViewportClear_matches_C2_3F_00_empty_count()
    {
        var pkt = PersonalShopWire602.BuildViewportClear();
        Assert.Equal(6, pkt.Length);
        Assert.Equal(0xC2, pkt[0]);
        Assert.Equal(0x3F, pkt[3]);
        Assert.Equal(0x00, pkt[4]);
        Assert.Equal(0x00, pkt[5]);
    }
}
