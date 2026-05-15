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
}
