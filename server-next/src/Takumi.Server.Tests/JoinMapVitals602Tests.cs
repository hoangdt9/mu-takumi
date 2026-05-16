using System.Buffers.Binary;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class JoinMapVitals602Tests
{
    [Fact]
    public void Join_uses_persisted_hp_mp_zen_when_max_set()
    {
        var name = new byte[10];
        "Hero"u8.CopyTo(name);
        var vitals = CharacterRosterVitals.FromInts(currentHp: 321, maxHp: 500, currentMp: 80, maxMp: 120, zen: 1_234_567);
        var pkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, serverClass: 0x20, level: 10, vitals));

        Assert.Equal(321, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(34)));
        Assert.Equal(500, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(36)));
        Assert.Equal(80, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(38)));
        Assert.Equal(120, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(40)));
        Assert.Equal(1_234_567u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(50)));
        Assert.Equal(321u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(79)));
        Assert.Equal(500u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(83)));
    }

    [Fact]
    public void Join_uses_persisted_shield_when_max_sd_set()
    {
        var name = new byte[10];
        "Sd"u8.CopyTo(name);
        var vitals = CharacterRosterVitals.FromInts(100, 200, 30, 60, zen: 0, currentShield: 40, maxShield: 180);
        var pkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, serverClass: 0x20, level: 10, vitals));

        Assert.Equal(40, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(42)));
        Assert.Equal(180, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(44)));
        Assert.Equal(40u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(103)));
        Assert.Equal(180u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(107)));
    }

    [Fact]
    public void Join_falls_back_to_stub_when_vitals_unset()
    {
        var name = new byte[10];
        "New"u8.CopyTo(name);
        var pkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, serverClass: 0x00, level: 5));

        var life = BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(34));
        var lifeMax = BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(36));
        Assert.True(life > 0);
        Assert.Equal(lifeMax, life);
        Assert.Equal(0u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(50)));
    }
}
