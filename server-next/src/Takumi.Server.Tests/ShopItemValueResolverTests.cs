using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ShopItemValueResolverTests
{
    [Fact]
    public void ResolveBuy_elfLala_pants_not_stub_price()
    {
        var item = new NpcShopItemEntry
        {
            ShopIndex = 10,
            Slot = 2,
            ItemGroup = 9,
            ItemIndex = 24,
            ItemLevel = 9,
            Durability = 0,
            Skill = 0,
            Luck = 1,
            Option = 7,
            ExcOpt = 63,
        };

        var price = ShopItemValueResolver.ResolveBuy(item);
        Assert.True(price > 100_000, $"expected legacy-scale price, got {price}");
        Assert.NotEqual(4880, price);
    }
}
