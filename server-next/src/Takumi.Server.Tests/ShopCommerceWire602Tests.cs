using System.Buffers.Binary;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ShopCommerceWire602Tests
{
    [Fact]
    public void BuildSell_matches_PRECEIVE_GOLD_layout()
    {
        var pkt = ShopCommerceWire602.BuildSell(1, 1_234_567);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(0x33, pkt[2]);
        Assert.Equal(1, pkt[3]);
        Assert.Equal(1_234_567u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(4)));
    }

    [Fact]
    public void BuildBuy_places_item_after_slot_index()
    {
        var item = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(item, 14, 3, 5, 40, false, false, 0, 0);
        var pkt = ShopCommerceWire602.BuildBuy(20, item);
        Assert.Equal(0x32, pkt[2]);
        Assert.Equal(20, pkt[3]);
        Assert.Equal(item[0], pkt[4]);
    }

    [Fact]
    public void BuildInventoryMoneyUpdate_uses_pick_zen_layout_big_endian()
    {
        var pkt = ItemWorldWire602.BuildInventoryMoneyUpdate(1_099_911_710);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(0x22, pkt[2]);
        Assert.Equal(ItemWorldWire602.PickZen, pkt[3]);
        Assert.Equal(0x41u, pkt[4]);
        Assert.Equal(0x8Fu, pkt[5]);
        Assert.Equal(0x52u, pkt[6]);
        Assert.Equal(0x1Eu, pkt[7]);
    }
}
