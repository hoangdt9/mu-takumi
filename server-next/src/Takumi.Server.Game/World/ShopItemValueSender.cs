using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

public static class ShopItemValueSender
{
    public static byte[] BuildForShop(IReadOnlyList<NpcShopItemEntry> shopItems, IReadOnlyDictionary<byte, byte[]> bagSlots)
    {
        ItemValueCatalog.EnsureInitialized();
        var entries = new List<ItemValueWire602.ItemValueEntry>();

        foreach (var item in shopItems)
        {
            var index = (item.ItemGroup * 512) + item.ItemIndex;
            if (!ItemValueCatalog.TryGetBuySell(index, item.ItemLevel, item.ExcOpt, out var buy, out var sell))
            {
                buy = (int)ShopItemPricing.BuyPrice(item);
                sell = Math.Max(1, buy / 3);
            }

            entries.Add(new ItemValueWire602.ItemValueEntry(index, item.ItemLevel, item.ExcOpt, 0, buy, 0, sell));
        }

        foreach (var (slot, blob) in bagSlots)
        {
            if (ItemWire602.IsEmpty(blob))
            {
                continue;
            }

            var index = blob[0] | ((blob[3] & 0x80) << 1);
            var level = (blob[1] >> 3) & 0x0F;
            var exc = blob[3] & 0x3F;
            if (!ItemValueCatalog.TryGetBuySell(index, level, exc, out var buy, out var sell))
            {
                sell = (int)ShopItemPricing.SellPrice(blob);
                buy = sell * 3;
            }

            entries.Add(new ItemValueWire602.ItemValueEntry(index, level, exc, 0, buy, 1, sell));
        }

        return ItemValueWire602.Build(entries);
    }
}
