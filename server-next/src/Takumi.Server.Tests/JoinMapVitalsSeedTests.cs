using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class JoinMapVitalsSeedTests
{
    [Fact]
    public void Seed_reads_stats_from_built_join_packet()
    {
        var name = new byte[10];
        "Seed"u8.CopyTo(name);
        var joinPkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, serverClass: 0x20, level: 15));

        Assert.True(JoinMapVitalsSeed.TryReadFromJoinPacket(joinPkt, out var v));
        Assert.True(v.MaxHp > 0);
        Assert.True(v.CurrentHp > 0);
        Assert.True(v.MaxMp > 0);
    }

    [Fact]
    public void Seed_skips_when_max_hp_already_set()
    {
        var name = new byte[10];
        "X"u8.CopyTo(name);
        var joinPkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, 0x20, 5));

        Assert.False(JoinMapVitalsSeed.TryApplyFromJoinPacketIfUnset(maxHpAlreadySet: true, joinPkt, out _));
    }

    [Fact]
    public void Seed_reads_shield_from_join_when_present_in_vitals()
    {
        var name = new byte[10];
        "Z"u8.CopyTo(name);
        var vitals = CharacterRosterVitals.FromInts(50, 100, 10, 20, 0, currentShield: 33, maxShield: 120);
        var joinPkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, 0x20, 8, vitals));
        Assert.True(JoinMapVitalsSeed.TryReadFromJoinPacket(joinPkt, out var v));
        Assert.Equal(33, v.CurrentShield);
        Assert.Equal(120, v.MaxShield);
    }
}
