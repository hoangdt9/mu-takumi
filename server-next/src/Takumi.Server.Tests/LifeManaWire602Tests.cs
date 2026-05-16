using System.Buffers.Binary;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class LifeManaWire602Tests
{
    [Fact]
    public void BuildLife_current_matches_client_layout()
    {
        var pkt = LifeManaWire602.BuildLife(LifeManaWire602.TypeCurrent, 321, shield: 50);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(LifeManaWire602.PacketLengthLife, pkt[1]);
        Assert.Equal(0x26, pkt[2]);
        Assert.Equal(LifeManaWire602.TypeCurrent, pkt[3]);
        Assert.Equal(321, (pkt[4] << 8) | pkt[5]);
        Assert.Equal(50, (pkt[7] << 8) | pkt[8]);
        Assert.Equal(321u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(9)));
        Assert.Equal(50u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(13)));
    }

    [Fact]
    public void BuildMana_max_includes_view_dwords_for_takumi_client()
    {
        var pkt = LifeManaWire602.BuildMana(LifeManaWire602.TypeMax, mana: 30, bp: 37);
        Assert.Equal(LifeManaWire602.PacketLengthMana, pkt[1]);
        Assert.Equal(0x27, pkt[2]);
        Assert.Equal(30, (pkt[4] << 8) | pkt[5]);
        Assert.Equal(37, (pkt[6] << 8) | pkt[7]);
        Assert.Equal(30u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(8)));
        Assert.Equal(37u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(12)));
    }

    [Fact]
    public void TryApplyVitalsFromOutbound_updates_hp_mp_and_shield()
    {
        var curHp = 0;
        var maxHp = 0;
        var curMp = 0;
        var maxMp = 0;
        var curSd = 0;
        var maxSd = 0;
        var buf = new byte[80];
        LifeManaWire602.BuildLife(LifeManaWire602.TypeCurrent, 100, 55).CopyTo(buf, 0);
        LifeManaWire602.BuildLife(LifeManaWire602.TypeMax, 200, 180).CopyTo(buf, 17);
        LifeManaWire602.BuildMana(LifeManaWire602.TypeMax, 220).CopyTo(buf, 34);

        Assert.True(LifeManaWire602.TryApplyVitalsFromOutbound(buf, ref curHp, ref maxHp, ref curMp, ref maxMp, ref curSd, ref maxSd));
        Assert.Equal(100, curHp);
        Assert.Equal(55, curSd);
        Assert.Equal(200, maxHp);
        Assert.Equal(180, maxSd);
        Assert.Equal(220, maxMp);
    }
}
