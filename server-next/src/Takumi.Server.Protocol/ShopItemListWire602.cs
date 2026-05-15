namespace Takumi.Server.Protocol;

/// <summary>Shop inventory (<c>PMSG_SHOP_ITEM_LIST_SEND</c> / <c>C2 0x31</c>, client <c>ReceiveTradeInventory</c>).</summary>
public static class ShopItemListWire602
{
    public const byte HeadCode = 0x31;

    public const int HeaderLength = 6;

    public const int EntryLength = 1 + Season6ItemWire602.ItemWireBytes;

    public static byte[] Build(IReadOnlyList<ShopWireItem> items)
    {
        if (items.Count == 0)
        {
            return BuildHeader(0);
        }

        var size = HeaderLength + (items.Count * EntryLength);
        var buf = new byte[size];
        buf[0] = 0xC2;
        buf[1] = (byte)((size >> 8) & 0xFF);
        buf[2] = (byte)(size & 0xFF);
        buf[3] = HeadCode;
        buf[4] = 0;
        buf[5] = (byte)items.Count;

        var o = HeaderLength;
        foreach (var item in items)
        {
            buf[o++] = item.Slot;
            item.Item12.CopyTo(buf.AsSpan(o, Season6ItemWire602.ItemWireBytes));
            o += Season6ItemWire602.ItemWireBytes;
        }

        return buf;
    }

    static byte[] BuildHeader(byte count)
    {
        return new byte[] { 0xC2, 0x00, HeaderLength, HeadCode, 0, count };
    }
}

public readonly record struct ShopWireItem(byte Slot, byte[] Item12);
