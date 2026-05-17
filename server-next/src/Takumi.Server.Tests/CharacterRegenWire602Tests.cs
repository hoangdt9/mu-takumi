using System.Buffers.Binary;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

/// <summary>Layout parity with client <c>PRECEIVE_REVIVAL</c> (PBMSG + SubCode + tile XY + map + angle + GAMESERVER_EXTRA).</summary>
public sealed class CharacterRegenWire602Tests
{
    [Fact]
    public void Build_matches_client_PRECEIVE_REVIVAL_offsets()
    {
        var town = JoinMapSpawnWire.LorenciaDefault;
        var pkt = CharacterRegenWire602.Build(
            town.Map,
            town.PositionX,
            town.PositionY,
            town.Angle,
            life: 500,
            mana: 200,
            shield: 100,
            bp: 50,
            experience: 0x0123456789ABCDEFUL,
            gold: 12345,
            viewCurHp: 500,
            viewCurMp: 200,
            viewCurBp: 50,
            viewCurSd: 100);

        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(CharacterRegenWire602.PacketLength, pkt[1]);
        Assert.Equal(0xF3, pkt[2]);
        Assert.Equal(0x04, pkt[3]);
        Assert.Equal(town.PositionX, pkt[4]);
        Assert.Equal(town.PositionY, pkt[5]);
        Assert.Equal(town.Map, pkt[6]);
        Assert.Equal(town.Angle, pkt[7]);
        Assert.Equal(500, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(8)));
        Assert.Equal(200, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(10)));
        Assert.Equal(100, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(12)));
        Assert.Equal(50, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(14)));
        Assert.Equal(0x0123456789ABCDEFUL, BinaryPrimitives.ReadUInt64LittleEndian(pkt.AsSpan(16)));
        Assert.Equal(12345u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(24)));
        Assert.Equal(500u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(28)));
        Assert.Equal(200u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(32)));
        Assert.Equal(50u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(36)));
        Assert.Equal(100u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(40)));
    }
}
