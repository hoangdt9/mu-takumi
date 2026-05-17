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
            var blob = new byte[ItemWire602.WireBytes];
            ItemWire602.WriteSeason6Item(
                blob,
                item.ItemGroup,
                item.ItemIndex,
                item.ItemLevel,
                item.Durability,
                item.Skill != 0,
                item.Luck != 0,
                item.Option,
                item.ExcOpt);
            entries.Add(ShopItemValueResolver.ToWireEntry(item, blob));
        }

        foreach (var (slot, blob) in bagSlots)
        {
            if (ItemWire602.IsEmpty(blob))
            {
                continue;
            }

            var index = ItemWire602.DecodeItemIndex(blob);
            var level = (blob[1] >> 3) & 0x0F;
            var exc = blob[3] & 0x3F;
            var sell = (int)Math.Clamp(ShopItemValueResolver.ResolveSell(blob), 0, int.MaxValue);
            var buy = (int)Math.Clamp(sell * 3L, 0, int.MaxValue);
            entries.Add(new ItemValueWire602.ItemValueEntry(index, level, exc, 0, buy, 1, sell));
        }

        return ItemValueWire602.Build(entries);
    }
}
