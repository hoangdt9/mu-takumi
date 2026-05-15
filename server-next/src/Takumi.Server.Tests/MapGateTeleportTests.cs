using Takumi.Server.Game;
using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapGateTeleportTests
{
    [Fact]
    public void TeleportWire602_matches_client_layout()
    {
        var pkt = TeleportWire602.Build(1, 1, 107, 247, 1);
        Assert.Equal(new byte[] { 0xC1, 9, 0x1C, 1, 0, 1, 107, 247, 1 }, pkt);
    }

    static string GateFixturePath() =>
        Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "Fixtures", "Gate.sample.txt"));

    [Fact]
    public void GateLoader_sample_gate1_is_in_range_at_trigger_tile()
    {
        var gate = GateLoader.LoadFromFile(GateFixturePath()).First(g => g.GateIndex == 1);
        Assert.True(MapGateTeleportService.IsPlayerInGate(gate, gate.MapId, (byte)gate.RangeTx, (byte)gate.RangeTy));
        Assert.False(MapGateTeleportService.IsPlayerInGate(gate, gate.MapId, 50, 50));
    }

    [Fact]
    public void TryTeleport_gate1_moves_to_map1_when_standing_at_gate()
    {
        var gates = GateLoader.LoadFromFile(GateFixturePath());
        MapGateCatalog.LoadForTests(gates);
        var gate = gates.First(g => g.GateIndex == 1);
        var result = MapGateTeleportService.TryTeleport(
            1,
            gate.MapId,
            (byte)gate.RangeTx,
            (byte)gate.RangeTy,
            0,
            level: 50);
        Assert.True(result.Accepted);
        Assert.Equal(1, result.MapId);
        Assert.Equal((ushort)1, result.ClientFlag);
    }

    [Fact]
    public void TryFindTeleportGateRequest_parses_c3_android_gate_packet()
    {
        ReadOnlySpan<byte> p = stackalloc byte[] { 0xC3, 0x06, 0x1C, 1, 120, 230 };
        Assert.True(GamePacketFinders.TryFindTeleportGateRequest(p, out var gate, out _, out _));
        Assert.Equal(1, gate);
    }
}
