using Takumi.Server.Game;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class DecryptedPacketSafety602Tests
{
    [Fact]
    public void ParseMaxDecryptedPacketBytes_in_allowed_range()
    {
        var v = DecryptedPacketSafety602.ParseMaxDecryptedPacketBytes();
        Assert.InRange(v, 512, 65535);
    }

    [Fact]
    public void PacketRateGate_allows_burst_then_blocks_in_same_window()
    {
        var g = new DecryptedPacketRateGate(2);
        var t0 = DateTimeOffset.Parse("2026-05-14T12:00:00Z", System.Globalization.CultureInfo.InvariantCulture);
        Assert.True(g.TryAllow(t0));
        Assert.True(g.TryAllow(t0.AddMilliseconds(1)));
        Assert.False(g.TryAllow(t0.AddMilliseconds(2)));
        Assert.True(g.TryAllow(t0.AddSeconds(1.1)));
    }

    [Fact]
    public void PacketRateGate_zero_always_allows()
    {
        var g = new DecryptedPacketRateGate(0);
        var t = DateTimeOffset.UtcNow;
        for (var i = 0; i < 20; i++)
        {
            Assert.True(g.TryAllow(t));
        }
    }
}
