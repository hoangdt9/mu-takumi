using System.Buffers.Binary;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ExperienceFormula602Tests
{
    [Theory]
    [InlineData(1, 100u)]
    [InlineData(2, 440u)]
    [InlineData(10, 19_000u)]
    public void Cumulative_matches_client_formula(int level, uint expected)
    {
        Assert.Equal(expected, ExperienceFormula602.CumulativeForLevel(level));
    }

    [Fact]
    public void Join_packet_seeds_next_experience_for_level()
    {
        var name = new byte[10];
        "dk002"u8.CopyTo(name);
        var pkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, serverClass: 0x20, level: 1));

        Assert.Equal(0UL, BinaryPrimitives.ReadUInt64LittleEndian(pkt.AsSpan(8)));
        Assert.Equal(100UL, BinaryPrimitives.ReadUInt64LittleEndian(pkt.AsSpan(16)));
    }
}
