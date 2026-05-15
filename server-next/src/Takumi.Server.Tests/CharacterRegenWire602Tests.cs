using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

/// <summary>Layout parity with client <c>PRECEIVE_REVIVAL</c> (PBMSG + SubCode + tile XY + map + angle).</summary>
public sealed class CharacterRegenWire602Tests
{
    [Fact]
    public void Build_matches_client_PRECEIVE_REVIVAL_offsets()
    {
        var town = JoinMapSpawnWire.LorenciaDefault;
        var pkt = CharacterRegenWire602.Build(town.Map, town.PositionX, town.PositionY, town.Angle, 500, 200);

        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(24, pkt[1]);
        Assert.Equal(0xF3, pkt[2]);
        Assert.Equal(0x04, pkt[3]);
        Assert.Equal(town.PositionX, pkt[4]);
        Assert.Equal(town.PositionY, pkt[5]);
        Assert.Equal(town.Map, pkt[6]);
        Assert.Equal(town.Angle, pkt[7]);
        Assert.Equal(0x01, pkt[8]);
        Assert.Equal(0xF4, pkt[9]);
        Assert.Equal(0x00, pkt[10]);
        Assert.Equal(0xC8, pkt[11]);
    }
}
