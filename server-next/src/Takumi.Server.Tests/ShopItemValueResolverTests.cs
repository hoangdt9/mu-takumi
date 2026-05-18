using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
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

    [Fact]
    public void ResolveBuy_elfLala_crossbow_exc63_matches_client_scale_not_300m()
    {
        ItemValueCatalog.EnsureInitialized();
        var item = new NpcShopItemEntry
        {
            ShopIndex = 10,
            Slot = 6,
            ItemGroup = 4,
            ItemIndex = 20,
            ItemLevel = 9,
            Durability = 0,
            Skill = 1,
            Luck = 1,
            Option = 7,
            ExcOpt = 63,
        };

        var index = (item.ItemGroup * 512) + item.ItemIndex;
        var price = ShopItemValueResolver.ResolveBuy(item);
        var charged = ShopItemValueResolver.ResolveChargedBuy(item, 0);
        var plain = new NpcShopItemEntry
        {
            ItemGroup = item.ItemGroup,
            ItemIndex = item.ItemIndex,
            ItemLevel = item.ItemLevel,
            Skill = item.Skill,
            Luck = item.Luck,
            Option = item.Option,
            ExcOpt = 0,
        };
        var noExc = ShopItemValueResolver.ResolveBuy(plain);
        Assert.True(
            price is > 35_000_000 and < 45_000_000,
            $"expected client-scale ~40M, got price={price} noExc={noExc}");
        Assert.Equal(price, ShopItemValueResolver.ToWireEntry(item, 0).Value);
        Assert.Equal(price, charged);
        Assert.Equal(charged, ShopItemValueResolver.ToWireEntry(item, 0).Value);
    }

    [Fact]
    public void ResolveBuy_large_healing_potion_stack_matches_client_itemvalue_buy()
    {
        var item = new NpcShopItemEntry
        {
            ShopIndex = 0,
            Slot = 0,
            ItemGroup = 14,
            ItemIndex = 3,
            ItemLevel = 0,
            Durability = 255,
            Skill = 0,
            Luck = 0,
            Option = 0,
            ExcOpt = 0,
        };

        // Client ItemValue(ip,1): 1500 * 255 / 3 = 127500 for ITEM_POTION+3 full stack.
        const int expectedBuy = 127_500;
        var buy = ShopItemValueResolver.ResolveBuy(item);
        var charged = ShopItemValueResolver.ResolveChargedBuy(item, 50);
        var wire = ShopItemValueResolver.ToWireEntry(item, 50).Value;
        Assert.True(buy > 0, $"buy={buy}");
        Assert.Equal(buy + (buy * 50 / 100), charged);
        Assert.Equal(charged, wire);
        Assert.Equal(expectedBuy, buy);
    }

    [Fact]
    public void ResolveBuy_exc_item_not_undercut_by_itemvalue_grade_wildcard()
    {
        ItemValueCatalog.EnsureInitialized();
        var item = new NpcShopItemEntry
        {
            ShopIndex = 6,
            Slot = 0,
            ItemGroup = 12,
            ItemIndex = 36,
            ItemLevel = 0,
            Durability = 255,
            Skill = 0,
            Luck = 1,
            Option = 7,
            ExcOpt = 63,
        };

        var index = (item.ItemGroup * 512) + item.ItemIndex;
        _ = ItemValueCatalog.TryGetBuySell(index, item.ItemLevel, item.ExcOpt, out var wildcardBuy, out _);
        var price = ShopItemValueResolver.ResolveBuy(item);
        if (wildcardBuy > 0)
        {
            Assert.True(price > wildcardBuy, $"resolve={price} wildcard={wildcardBuy}");
        }
    }

    [Fact]
    public void ToWireEntry_sellValue_matches_ResolveSell_on_same_shop_blob()
    {
        ItemValueCatalog.EnsureInitialized();
        var item = new NpcShopItemEntry
        {
            ShopIndex = 10,
            Slot = 6,
            ItemGroup = 4,
            ItemIndex = 20,
            ItemLevel = 9,
            Durability = 0,
            Skill = 1,
            Luck = 1,
            Option = 7,
            ExcOpt = 63,
        };

        var blob = new byte[ItemWire602.WireBytes];
        ShopItemWireEncoding.WriteShopEntry(blob, item);
        var expectedSell = (int)ShopItemValueResolver.ResolveSell(blob);
        var entry = ShopItemValueResolver.ToWireEntry(item, 0, blob);
        Assert.Equal((int)ShopItemValueResolver.ResolveChargedBuy(item, 0), entry.Value);
        Assert.Equal(expectedSell, entry.SellValue);
    }
}
