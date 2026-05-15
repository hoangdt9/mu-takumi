using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>NPC shop inventory (<c>C2 0x31</c> / <c>GCShopItemListSend</c>).</summary>
public static class NpcShopWire602
{
    public const byte HeadCode = 0x31;
    const int HeaderLength = 6;

    public readonly record struct ShopItemWire(byte Slot, ReadOnlyMemory<byte> Item12);

    public static byte[] Build(IReadOnlyList<ShopItemWire> items, byte listType = 0)
    {
        if (items.Count > 255)
        {
            throw new ArgumentOutOfRangeException(nameof(items), "Shop count must fit in a byte.");
        }

        var size = HeaderLength + (items.Count * (1 + ItemWire602.WireBytes));
        var buf = new byte[size];
        buf[0] = 0xC2;
        BinaryPrimitives.WriteUInt16BigEndian(buf.AsSpan(1, 2), (ushort)size);
        buf[3] = HeadCode;
        buf[4] = listType;
        buf[5] = (byte)items.Count;

        var o = HeaderLength;
        foreach (var item in items)
        {
            buf[o++] = item.Slot;
            item.Item12.Span.CopyTo(buf.AsSpan(o, ItemWire602.WireBytes));
            o += ItemWire602.WireBytes;
        }

        return buf;
    }
}
