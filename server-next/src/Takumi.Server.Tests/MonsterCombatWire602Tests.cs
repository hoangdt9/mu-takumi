using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterCombatWire602Tests
{
    [Fact]
    public void Hit_request_C1_11_parsed()
    {
        var p = new byte[] { 0xC1, 0x07, 0x11, 0x2E, 0xE1, 0x02, 0x04 };
        Assert.True(ClientHitPackets602.TryFindHitRequest(p, out _, out var id, out var anim, out var dir));
        Assert.Equal(12001, id);
        Assert.Equal(2, anim);
        Assert.Equal(4, dir);
    }

    [Fact]
    public void Damage_wire_C1_11_layout()
    {
        var pkt = MonsterDamageWire602.Build(12001, 50, 50, hitSuccess: true);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(0x11, pkt[2]);
        Assert.Equal(0xAE, pkt[3]);
        Assert.Equal(0xE1, pkt[4]);
        Assert.Equal(50, pkt[6]);
    }

    [Fact]
    public void Destroy_wire_C1_14_layout()
    {
        var pkt = MonsterViewportDestroyWire602.Build([12001]);
        Assert.Equal(new byte[] { 0xC1, 0x06, 0x14, 0x01, 0x2E, 0xE1 }, pkt);
    }

    [Fact]
    public void Die_wire_C1_16_layout()
    {
        var pkt = MonsterDieWire602.Build(12001, 100, 50);
        Assert.Equal(0x16, pkt[2]);
        Assert.Equal(0xAE, pkt[3]);
    }
}
