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
        Assert.Equal(0x26, pkt[2]);
        Assert.Equal(LifeManaWire602.TypeCurrent, pkt[3]);
        Assert.Equal(321, (pkt[4] << 8) | pkt[5]);
        Assert.Equal(50, (pkt[7] << 8) | pkt[8]);
    }

    [Fact]
    public void TryApplyVitalsFromOutbound_updates_hp_mp()
    {
        var curHp = 0;
        var maxHp = 0;
        var curMp = 0;
        var maxMp = 0;
        var buf = new byte[32];
        LifeManaWire602.BuildLife(LifeManaWire602.TypeCurrent, 100).CopyTo(buf, 0);
        LifeManaWire602.BuildMana(LifeManaWire602.TypeMax, 200).CopyTo(buf, 9);

        Assert.True(LifeManaWire602.TryApplyVitalsFromOutbound(buf, ref curHp, ref maxHp, ref curMp, ref maxMp));
        Assert.Equal(100, curHp);
        Assert.Equal(200, maxMp);
    }
}
