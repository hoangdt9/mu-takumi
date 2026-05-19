using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CrywolfWire602Tests
{
    [Fact]
    public void BuildInfo_peace_none()
    {
        var pkt = CrywolfWire602.BuildInfo(CrywolfWire602.OccupationPeace, CrywolfWire602.StateNone);
        Assert.Equal(new byte[] { 0xC1, 0x06, 0xBD, 0x00, 0x00, 0x00 }, pkt);
    }

    [Fact]
    public void TryFindInfoRequest_matches_client_request()
    {
        var pkt = new byte[] { 0xC1, 0x04, 0xBD, 0x00 };
        Assert.True(CrywolfWire602.TryFindInfoRequest(pkt, out var off));
        Assert.Equal(0, off);
    }
}
