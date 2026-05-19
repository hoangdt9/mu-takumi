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

        PlayerWarehouseSession.Open(id);
        Assert.True(PlayerUiSession.IsMoveBlocked(id));
        PlayerWarehouseSession.Close(id);
        Assert.False(PlayerUiSession.IsMoveBlocked(id));

        PlayerUiSession.Clear(id);
        PlayerUiSession.AddGenericInterface(id);
        Assert.True(PlayerUiSession.IsMoveBlocked(id));

        PlayerUiSession.RemoveGenericInterface(id);
        Assert.False(PlayerUiSession.IsMoveBlocked(id));
    }

    [Fact]
    public void IsWarehouseOnlyMoveBlock_true_when_only_warehouse_flag()
    {
        var id = Guid.NewGuid();
        Assert.False(PlayerUiSession.IsWarehouseOnlyMoveBlock(id));

        PlayerUiSession.SetWarehouse(id, true);
        Assert.True(PlayerUiSession.IsWarehouseOnlyMoveBlock(id));

        PlayerUiSession.SetTrade(id, true);
        Assert.False(PlayerUiSession.IsWarehouseOnlyMoveBlock(id));
    }
}
