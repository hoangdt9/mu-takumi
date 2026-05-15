using Takumi.Server.Game;
using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class NpcShopWireTests
{
    [Fact]
    public void NpcTalkWire602_builds_c1_30_shop_open()
    {
        var pkt = NpcTalkWire602.BuildShopOpen();
        Assert.Equal(new byte[] { 0xC1, 11, 0x30, 0, 0, 0, 0, 0, 0, 0, 0 }, pkt);
    }

    [Fact]
    public void ShopItemListWire602_builds_c2_header_and_one_item()
    {
        var item12 = new byte[12];
        Season6ItemWire602.EncodeShopItem(item12, 0, 20, 9, 0, 1, 1, 7, 63, 0, 0, 0, 255, 255, 255, 255, 255);
        var pkt = ShopItemListWire602.Build([new ShopWireItem(0, item12)]);
        Assert.Equal(0xC2, pkt[0]);
        Assert.Equal(0x31, pkt[3]);
        Assert.Equal(1, pkt[5]);
        Assert.Equal(0, pkt[6]);
        Assert.Equal(item12, pkt.AsSpan(7, 12).ToArray());
    }

    [Fact]
    public void TryFindNpcTalkRequest_parses_c1_30()
    {
        ReadOnlySpan<byte> p = stackalloc byte[] { 0xC1, 0x05, 0x30, 0x39, 0x30 };
        Assert.True(GamePacketFinders.TryFindNpcTalkRequest(p, out var key));
        Assert.Equal(0x3039, key);
    }

    [Fact]
    public void ResolveShopIndex_prefers_exact_map_tile()
    {
        NpcShopCatalog.LoadForTests(
            [
                new NpcShopEntry { ShopIndex = 0, MonsterClass = 251, MapId = null, PosX = null, PosY = null },
                new NpcShopEntry { ShopIndex = 1, MonsterClass = 255, MapId = 0, PosX = 120, PosY = 130 },
            ],
            []);
        Assert.Equal(1, NpcShopCatalog.ResolveShopIndex(255, 0, 120, 130));
        Assert.Equal(0, NpcShopCatalog.ResolveShopIndex(251, 0, 120, 130));
    }
}
