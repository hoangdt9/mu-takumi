using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class NewCharacterCalcWire602Tests
{
    [Fact]
    public void Build_DarkKnightLv17_HasNonZeroPhysiDamage()
    {
        var name = "dk001"u8.ToArray();
        var sheet = CharacterSheetStats.FromInts(28, 20, 25, 10, 0, 80);
        var roster = new CharacterRosterWire(name, serverClass: 0x20, level: 17, sheet: sheet);

        var pkt = NewCharacterCalcWire602.Build(roster);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(NewCharacterCalcWire602.PacketLength, pkt[1]);
        Assert.Equal(0xF3, pkt[2]);
        Assert.Equal(0xE1, pkt[3]);

        var physMin = BitConverter.ToUInt32(pkt, 4 + 8 * 4 + 5 * 4);
        var physMax = BitConverter.ToUInt32(pkt, 4 + 8 * 4 + 6 * 4);
        Assert.True(physMin > 0, "physMin");
        Assert.True(physMax >= physMin, "physMax");
    }
}
