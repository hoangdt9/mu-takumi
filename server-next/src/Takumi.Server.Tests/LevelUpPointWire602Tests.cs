using System.Buffers.Binary;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class LevelUpPointWire602Tests
{
    [Fact]
    public void BuildSuccess_fits_packet_length_and_writes_leadership()
    {
        var sheet = new CharacterSheetStats
        {
            Strength = 32,
            Dexterity = 27,
            Vitality = 25,
            Energy = 20,
            Leadership = 0,
            LevelUpPoint = 48,
        };
        var vitals = new CharacterComputedVitals
        {
            LifeMax = 120,
            ManaMax = 61,
            SkillManaMax = 37,
            ShieldMax = 0,
        };

        var pkt = LevelUpPointWire602.BuildSuccess(0x02, sheet, vitals, maxLifeOrMana: 108);

        Assert.Equal(51, pkt.Length);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(51, pkt[1]);
        Assert.Equal(0xF3, pkt[2]);
        Assert.Equal(0x06, pkt[3]);
        Assert.Equal(0x12, pkt[4]); // 16 | 2
        Assert.Equal(48u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(11)));
        Assert.Equal(32u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(31)));
        Assert.Equal(25u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(39)));
        Assert.Equal(0u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(47)));
    }
}
