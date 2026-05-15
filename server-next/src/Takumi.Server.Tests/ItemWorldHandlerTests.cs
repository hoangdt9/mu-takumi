using System.Text;
using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ItemWorldHandlerTests
{
    [Fact]
    public void TryFindItemMoveRequest_parses_C1_0x24_layout()
    {
        var pkt = new byte[ClientGameplayPackets602.ItemMoveFrameLength];
        pkt[0] = 0xC1;
        pkt[1] = (byte)ClientGameplayPackets602.ItemMoveFrameLength;
        pkt[2] = 0x24;
        pkt[3] = 0;
        pkt[4] = 20;
        pkt[15] = 0;
        pkt[18] = 30;

        Assert.True(ClientGameplayPackets602.TryFindItemMoveRequest(pkt, out _, out var sf, out var ss, out var tf, out var ts));
        Assert.Equal(0, sf);
        Assert.Equal((byte)20, ss);
        Assert.Equal(0, tf);
        Assert.Equal((byte)30, ts);
    }

    [Fact]
    public void TryMoveInventorySlot_swaps_two_bag_slots()
    {
        var sid = Guid.NewGuid();
        var a = new byte[ItemWire602.WireBytes];
        var b = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(a, 14, 3, 5, 40, false, false, 0, 0);
        ItemWire602.WriteSeason6Item(b, 14, 4, 6, 50, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 12, a);
        PlayerShopSession.SetSlot(sid, 13, b);

        Assert.True(PlayerShopSession.TryMoveInventorySlot(sid, 12, 13, out var at13));
        Assert.True(PlayerShopSession.TryGetSlot(sid, 12, out var now12));
        Assert.Equal(b[0], now12[0]);
        Assert.Equal(a[0], at13[0]);
    }

    [Fact]
    public void MapGroundItemStore_drop_and_take_roundtrip()
    {
        var item = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(item, 14, 3, 1, 10, false, false, 0, 0);
        var idx = MapGroundItemStore.Drop(0, 100, 100, item);
        Assert.True(idx > 0);
        Assert.True(MapGroundItemStore.TryTake(0, idx, 100, 100, out var picked));
        Assert.Equal(item[0], picked[0]);
        Assert.False(MapGroundItemStore.TryTake(0, idx, 100, 100, out _));
    }

    [Fact]
    public void ItemWorldWire602_move_fail_uses_0xFF_subcode()
    {
        var pkt = ItemWorldWire602.BuildMoveFail(22);
        Assert.Equal(0xFF, pkt[3]);
        Assert.Equal((byte)22, pkt[4]);
    }

    [Fact]
    public void PlayerDieWire602_layout_matches_GCUserDieSend()
    {
        var pkt = PlayerDieWire602.Build(victimObjectKey: 0x1234, killerObjectKey: 0x5678, skill: 0);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(PlayerDieWire602.PacketLength, pkt[1]);
        Assert.Equal((byte)0x17, pkt[2]);
        Assert.Equal(0x12, pkt[3]);
        Assert.Equal(0x34, pkt[4]);
        Assert.Equal(0x56, pkt[7]);
        Assert.Equal(0x78, pkt[8]);
    }

    [Fact]
    public void ItemViewportWire602_create_sets_fresh_drop_high_bit()
    {
        var item = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(item, 14, 3, 1, 10, false, false, 0, 0);
        var pkt = ItemViewportWire602.BuildCreateSingle(42, 10, 11, item, freshDrop: true);
        Assert.Equal(0x20, pkt[3]);
        Assert.Equal(1, pkt[4]);
        Assert.Equal(0x80 | 0, pkt[5]);
        Assert.Equal(42, pkt[6]);
        Assert.Equal((byte)10, pkt[7]);
        Assert.Equal((byte)11, pkt[8]);
    }
}
