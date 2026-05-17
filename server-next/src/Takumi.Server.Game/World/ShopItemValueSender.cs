using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

public static class ShopItemValueSender
{
    /// <summary>
    /// <c>C2 F3 E9</c> buy/sell zen for NPC shop stock only.
    /// Do not merge player bag rows here — same index/level/exc would overwrite shop buy with sell×3 and tooltips show ~1B instead of ~1M.
    /// </summary>
    public static byte[] BuildForShop(IReadOnlyList<NpcShopItemEntry> shopItems, int taxRatePercent)
    {
        ItemValueCatalog.EnsureInitialized();
        var entries = new List<ItemValueWire602.ItemValueEntry>(shopItems.Count);

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
            entries.Add(ShopItemValueResolver.ToWireEntry(item, taxRatePercent, blob));
        }

        return ItemValueWire602.Build(entries);
    }
}
