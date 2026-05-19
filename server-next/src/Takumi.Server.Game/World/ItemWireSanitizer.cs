using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>
/// Clears socket columns on non-socket item types before wire send / persist
/// (parity <c>SocketItemType.txt</c> + client <c>CSocketItemMgr::IsSocketItem(int)</c>).
/// </summary>
public static class ItemWireSanitizer
{
    public static void NormalizeSocketEncoding(Span<byte> item12)
    {
        if (item12.Length < ItemWire602.WireBytes || ItemWire602.IsEmpty(item12))
        {
            return;
        }

        var index = ItemWire602.DecodeItemIndex(item12);
        if (index < 0)
        {
            return;
        }

        if (SocketItemTypeCatalog.IsSocketItem(index / 512, index % 512, out _))
        {
            return;
        }

        var joh = item12[6];
        if (joh is ItemWire602.NoSocket or ItemWire602.EmptySocket)
        {
            joh = 0;
        }

        item12[6] = joh;
        item12.Slice(7, 5).Fill(ItemWire602.NoSocket);
    }

    public static void NormalizeSocketEncoding(IDictionary<byte, byte[]> slots)
    {
        SocketItemTypeCatalog.EnsureInitialized();
        foreach (var key in slots.Keys.ToArray())
        {
            var blob = slots[key];
            if (blob.Length < ItemWire602.WireBytes)
            {
                continue;
            }

            NormalizeSocketEncoding(blob);
            slots[key] = blob;
        }
    }
}
