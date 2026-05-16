using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ExperienceProgression602Tests
{
    [Fact]
    public void Kill_exp_levels_up_and_grants_stat_points()
    {
        ushort level = 1;
        uint exp = 0;
        ushort statPts = 0;
        var levels = ExperienceProgression602.ApplyKillExperience(
            ref level,
            ref exp,
            ref statPts,
            serverClass: 0x20,
            expGain: 100);

        Assert.Equal(1, levels);
        Assert.Equal(2, level);
        Assert.Equal(100u, exp);
        Assert.Equal(5, statPts);
    }

    [Fact]
    public void Join_wire_uses_persisted_experience()
    {
        var name = new byte[10];
        "dk001"u8.CopyTo(name);
        var pkt = JoinMapServerWire602.Build(
            new CharacterRosterWire(name, serverClass: 0x20, level: 2, experience: 150));

        Assert.Equal(150UL, System.Buffers.Binary.BinaryPrimitives.ReadUInt64LittleEndian(pkt.AsSpan(8)));
        Assert.Equal(440UL, System.Buffers.Binary.BinaryPrimitives.ReadUInt64LittleEndian(pkt.AsSpan(16)));
    }
}
