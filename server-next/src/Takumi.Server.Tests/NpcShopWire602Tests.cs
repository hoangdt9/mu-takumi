using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class NpcShopWire602Tests
{
    [Fact]
    public void Build_encodes_header_and_one_item()
    {
        var item = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(item, itemGroup: 7, itemIndex: 5, level: 3, durability: 45, skill: false, luck: true, option: 1, excellent: 0);
        var pkt = NpcShopWire602.Build([new NpcShopWire602.ShopItemWire(0, item)]);

        Assert.Equal(0xC2, pkt[0]);
        Assert.Equal(0x31, pkt[3]);
        Assert.Equal(0, pkt[4]);
        Assert.Equal(1, pkt[5]);
        Assert.Equal(0, pkt[6]);
        Assert.Equal(item[0], pkt[7]);
    }
}
