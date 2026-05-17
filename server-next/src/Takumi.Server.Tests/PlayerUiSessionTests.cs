using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class PlayerUiSessionTests
{
    [Fact]
    public void IsMoveBlocked_reflects_shop_warehouse_trade_and_personal_shop()
    {
        var id = Guid.NewGuid();
        Assert.False(PlayerUiSession.IsMoveBlocked(id));

        PlayerUiSession.SetPersonalShop(id, true);
        Assert.True(PlayerUiSession.IsMoveBlocked(id));

        PlayerUiSession.SetPersonalShop(id, false);
        PlayerUiSession.SetNpcShop(id, true);
        Assert.True(PlayerUiSession.IsMoveBlocked(id));

        PlayerUiSession.Clear(id);
        PlayerUiSession.SetWarehouse(id, true);
        Assert.True(PlayerUiSession.IsMoveBlocked(id));

        PlayerUiSession.Clear(id);
        PlayerUiSession.AddGenericInterface(id);
        Assert.True(PlayerUiSession.IsMoveBlocked(id));

        PlayerUiSession.RemoveGenericInterface(id);
        Assert.False(PlayerUiSession.IsMoveBlocked(id));
    }
}
