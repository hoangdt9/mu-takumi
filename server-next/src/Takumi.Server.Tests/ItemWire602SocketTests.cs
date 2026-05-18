using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ItemWire602SocketTests
{
    [Fact]
    public void WriteShopItem_non_socket_item_uses_harmony_and_no_socket_bytes()
    {
        var blob = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteShopItem(
            blob,
            itemGroup: 0,
            itemIndex: 20,
            level: 5,
            durability: 40,
            skill: false,
            luck: false,
            option: 0,
            excellent: 0,
            ancient: 0,
            jewelOfHarmony: 12,
            itemOptionEx: 0,
            isSocketItem: false,
            maxSocket: 0,
            socket1: 10,
            socket2: 20,
            socket3: 30,
            socket4: 40,
            socket5: 50);

        Assert.Equal((byte)12, blob[6]);
        Assert.All(blob[7..12], b => Assert.Equal(ItemWire602.NoSocket, b));
    }

    [Fact]
    public void WriteShopItem_socket_item_applies_columns_up_to_max_socket()
    {
        var blob = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteShopItem(
            blob,
            itemGroup: 0,
            itemIndex: 26,
            level: 0,
            durability: 255,
            skill: false,
            luck: false,
            option: 0,
            excellent: 0,
            ancient: 0,
            jewelOfHarmony: 99,
            itemOptionEx: 0,
            isSocketItem: true,
            maxSocket: 3,
            socket1: 1,
            socket2: 2,
            socket3: 3,
            socket4: 4,
            socket5: 5);

        Assert.Equal(ItemWire602.NoSocket, blob[6]);
        Assert.Equal((byte)1, blob[7]);
        Assert.Equal((byte)2, blob[8]);
        Assert.Equal((byte)3, blob[9]);
        Assert.Equal(ItemWire602.NoSocket, blob[10]);
        Assert.Equal(ItemWire602.NoSocket, blob[11]);
    }

    [Fact]
    public void ShopItemWireEncoding_gates_socket_columns_by_catalog()
    {
        SocketItemTypeCatalog.LoadForTests(new Dictionary<int, int> { [(0 * 512) + 26] = 5 });

        var shop = new NpcShopItemEntry
        {
            ItemGroup = 0,
            ItemIndex = 26,
            ItemLevel = 0,
            Durability = 255,
            Joh = 77,
            Socket1 = 11,
            Socket2 = 22,
            Socket3 = 33,
            Socket4 = 44,
            Socket5 = 55,
        };

        var blob = new byte[ItemWire602.WireBytes];
        ShopItemWireEncoding.WriteShopEntry(blob, shop);
        Assert.Equal((byte)11, blob[7]);
        Assert.Equal((byte)55, blob[11]);

        var normal = new NpcShopItemEntry
        {
            ItemGroup = 0,
            ItemIndex = 20,
            ItemLevel = 0,
            Durability = 40,
            Joh = 88,
            Socket1 = 11,
        };
        ShopItemWireEncoding.WriteShopEntry(blob, normal);
        Assert.Equal((byte)88, blob[6]);
        Assert.All(blob[7..12], b => Assert.Equal(ItemWire602.NoSocket, b));
    }
}
