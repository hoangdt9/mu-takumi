using Takumi.Server.Game;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class PresenceBroadcastRateGateTests
{
    [Fact]
    public void DecryptedPacketRateGate_limits_burst()
    {
        var gate = new DecryptedPacketRateGate(2);
        var t0 = DateTimeOffset.UtcNow;
        Assert.True(gate.TryAllow(t0));
        Assert.True(gate.TryAllow(t0.AddMilliseconds(10)));
        Assert.False(gate.TryAllow(t0.AddMilliseconds(20)));
    }
}
