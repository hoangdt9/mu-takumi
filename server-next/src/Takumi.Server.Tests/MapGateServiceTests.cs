using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapGateServiceTests
{
    [Fact]
    public void TryResolveGateTeleport_follows_target_gate_chain()
    {
        MapGateCatalog.EnsureInitialized();
        if (!MapGateCatalog.TryGetGate(1, out _))
        {
            return;
        }

        Environment.SetEnvironmentVariable("TAKUMI_GATE_SKIP_PROXIMITY", "1");

        var ok = MapGateService.TryResolveGateTeleport(
            1,
            playerMap: 0,
            playerX: 123,
            playerY: 233,
            playerLevel: 50,
            previousMap: 0,
            out var dest);

        Assert.True(ok);
        Assert.Equal(1, dest.MapId);
    }

    [Fact]
    public void TeleportWire602_matches_client_layout()
    {
        var pkt = TeleportWire602.Build(1, 0, 120, 130, 2);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(TeleportWire602.PacketLength, pkt[1]);
        Assert.Equal(0x1C, pkt[2]);
        Assert.Equal(1, pkt[3]);
        Assert.Equal(0, pkt[4]);
        Assert.Equal(0, pkt[5]);
        Assert.Equal(120, pkt[6]);
        Assert.Equal(130, pkt[7]);
        Assert.Equal(2, pkt[8]);
    }
}
