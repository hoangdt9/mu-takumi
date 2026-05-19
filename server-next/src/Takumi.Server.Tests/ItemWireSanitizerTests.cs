using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ItemWireSanitizerTests
{
    [Fact]
    public void NormalizeSocketEncoding_clears_socket_columns_on_wing()
    {
        SocketItemTypeCatalog.LoadForTests(new Dictionary<int, int> { [(0 * 512) + 26] = 5 });
        var wing = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(wing, 12, 39, 0, 200, false, false, 0, 0);
        wing[7] = 0;
        wing[8] = 10;
        wing[9] = 20;

        ItemWireSanitizer.NormalizeSocketEncoding(wing);

        Assert.Equal(ItemWire602.NoSocket, wing[7]);
        Assert.Equal(ItemWire602.NoSocket, wing[8]);
        Assert.Equal(ItemWire602.NoSocket, wing[9]);
    }

    [Fact]
    public void NormalizeSocketEncoding_preserves_socket_weapon_columns()
    {
        SocketItemTypeCatalog.LoadForTests(new Dictionary<int, int> { [(0 * 512) + 26] = 5 });
        var sword = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteShopItem(
            sword,
            0,
            26,
            0,
            40,
            false,
            false,
            0,
            0,
            0,
            0,
            0,
            isSocketItem: true,
            maxSocket: 5,
            socket1: 0,
            socket2: 10,
            socket3: 20,
            socket4: 30,
            socket5: 40);

        ItemWireSanitizer.NormalizeSocketEncoding(sword);

        Assert.Equal((byte)0, sword[7]);
        Assert.Equal((byte)10, sword[8]);
    }
}
