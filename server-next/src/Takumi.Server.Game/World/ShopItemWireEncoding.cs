using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Builds 12-byte shop item blobs with socket gating (parity <c>CShop::InsertItemNew</c>).</summary>
public static class ShopItemWireEncoding
{
    public static void WriteShopEntry(Span<byte> dest, NpcShopItemEntry item)
    {
        var isSocket = SocketItemTypeCatalog.IsSocketItem(item.ItemGroup, item.ItemIndex, out var maxSocket);
        ItemWire602.WriteShopItem(
            dest,
            new ItemWire602.ShopItemWireSource(
                item.ItemGroup,
                item.ItemIndex,
                item.ItemLevel,
                item.Durability,
                item.Skill != 0,
                item.Luck != 0,
                item.Option,
                item.ExcOpt,
                item.Anc,
                item.Joh,
                item.Oex,
                isSocket,
                maxSocket,
                item.Socket1,
                item.Socket2,
                item.Socket3,
                item.Socket4,
                item.Socket5));
    }
}
