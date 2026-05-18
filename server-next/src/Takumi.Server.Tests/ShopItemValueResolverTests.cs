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
    public void ResolveSell_exc63_elf_pants_not_wildcard_undercut()
    {
        ItemValueCatalog.EnsureInitialized();
        var item = new NpcShopItemEntry
        {
            ShopIndex = 10,
            Slot = 2,
            ItemGroup = 9,
            ItemIndex = 24,
            ItemLevel = 9,
            Durability = 255,
            Skill = 0,
            Luck = 1,
            Option = 7,
            ExcOpt = 63,
        };

        var index = (item.ItemGroup * 512) + item.ItemIndex;
        _ = ItemValueCatalog.TryGetBuySell(index, item.ItemLevel, item.ExcOpt, out _, out var wildcardSell);
        var blob = new byte[ItemWire602.WireBytes];
        ShopItemWireEncoding.WriteShopEntry(blob, item);
        var buy = ShopItemValueResolver.ResolveBuy(item);
        var sell = ShopItemValueResolver.ResolveSell(blob);

        // Wildcard ItemValue row credited ~4.3M; legacy buy/3 is ~5.3M and matches F3 E9 + 0x32 sell.
        if (wildcardSell > 0)
        {
            Assert.True(sell > wildcardSell, $"sell={sell} wildcardSell={wildcardSell}");
        }

        Assert.InRange(sell, buy / 5, buy / 2);
        Assert.Equal(sell, ShopItemValueResolver.ResolveSell(blob));
    }

    [Fact]
    public void ResolveSell_socket_boots_higher_than_plain_itemvalue_row()
    {
        ItemValueCatalog.EnsureInitialized();
        const int index = (9 * 512) + 23;
        var plain = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(plain, 9, 23, 7, 109, skill: true, luck: true, option: 7, excellent: 0);

        var socketed = new byte[ItemWire602.WireBytes];
        ShopItemWireEncoding.WriteShopEntry(
            socketed,
            new NpcShopItemEntry
            {
                ItemGroup = 9,
                ItemIndex = 23,
                ItemLevel = 7,
                Durability = 109,
                Skill = 1,
                Luck = 1,
                Option = 7,
                ExcOpt = 63,
                Socket1 = 0,
                Socket2 = 1,
                Socket3 = 2,
                Socket4 = 3,
                Socket5 = 4,
            });

        var plainSell = ShopItemValueResolver.ResolveSell(plain);
        var socketSell = ShopItemValueResolver.ResolveSell(socketed);

        Assert.True(socketSell > plainSell * 2, $"socket={socketSell} plain={plainSell}");
        Assert.InRange(socketSell, 3_000_000, 25_000_000);
    }

    [Fact]
    public void ResolveSell_exc_zero_uses_itemvalue_wildcard_exc_nonzero_uses_legacy()
    {
        ItemValueCatalog.EnsureInitialized();
        const int index = (9 * 512) + 23;
        var blobExc0 = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(blobExc0, 9, 23, 7, 109, skill: true, luck: true, option: 7, excellent: 0);
        var blobExc63 = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(blobExc63, 9, 23, 7, 109, skill: true, luck: true, option: 7, excellent: 63);

        var sellPlain = ShopItemValueResolver.ResolveSell(blobExc0);
        var sellExc = ShopItemValueResolver.ResolveSell(blobExc63);
        var legacySell = LegacyShopBuyPriceEstimate.Estimate(index, 7, 63, 7, true, skill: true) / 3;

        Assert.True(sellExc >= legacySell / 2, $"exc sell={sellExc} legacy/3={legacySell}");
        Assert.NotEqual(sellPlain, sellExc);
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
