namespace Takumi.Server.Protocol;

/// <summary>Season 6 ground item viewport (<c>C2 0x20</c> / <c>0x21</c>).</summary>
public static class ItemViewportWire602
{
    public const byte CreateHead = 0x20;
    public const byte DeleteHead = 0x21;
    public const int EntryBytes = 16;

    public static byte[] BuildCreateSingle(ushort mapItemIndex, byte x, byte y, ReadOnlySpan<byte> item12, bool freshDrop = true)
    {
        var size = 4 + 1 + EntryBytes;
        var buf = new byte[size];
        buf[0] = 0xC2;
        buf[1] = (byte)((size >> 8) & 0xFF);
        buf[2] = (byte)(size & 0xFF);
        buf[3] = CreateHead;
        buf[4] = 1;

        var key = mapItemIndex & 0x7FFF;
        var keyH = (byte)((key >> 8) & 0xFF);
        var keyL = (byte)(key & 0xFF);
        if (freshDrop)
        {
            keyH |= 0x80;
        }

        buf[5] = keyH;
        buf[6] = keyL;
        buf[7] = x;
        buf[8] = y;
        item12.CopyTo(buf.AsSpan(9, ItemWire602.WireBytes));
        return buf;
    }

    public static byte[] BuildDeleteSingle(ushort mapItemIndex)
    {
        var size = 4 + 1 + 2;
        var buf = new byte[size];
        buf[0] = 0xC2;
        buf[1] = (byte)((size >> 8) & 0xFF);
        buf[2] = (byte)(size & 0xFF);
        buf[3] = DeleteHead;
        buf[4] = 1;
        var key = mapItemIndex & 0x7FFF;
        buf[5] = (byte)((key >> 8) & 0xFF);
        buf[6] = (byte)(key & 0xFF);
        return buf;
    }
}
