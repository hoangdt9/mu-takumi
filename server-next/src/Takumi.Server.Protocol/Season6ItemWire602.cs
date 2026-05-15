namespace Takumi.Server.Protocol;

/// <summary>12-byte Season 6 item blob (parity <c>CItemManager::ItemByteConvert</c>).</summary>
public static class Season6ItemWire602
{
    public const int ItemWireBytes = 12;

    const int MaxItemType = 512;

    public static int ComposeItemIndex(int itemGroup, int itemIndex) => (itemGroup * MaxItemType) + itemIndex;

    public static void EncodeShopItem(
        Span<byte> dest12,
        int itemGroup,
        int itemIndex,
        int level,
        int durability,
        int skill,
        int luck,
        int option,
        int excOpt,
        int anc,
        int joh,
        int oex,
        int socket1,
        int socket2,
        int socket3,
        int socket4,
        int socket5)
    {
        if (dest12.Length < ItemWireBytes)
        {
            throw new ArgumentException("Destination span too small.", nameof(dest12));
        }

        var index = ComposeItemIndex(itemGroup, itemIndex);
        dest12[0] = (byte)(index & 0xFF);

        dest12[1] = 0;
        dest12[1] |= (byte)((level * 8) & 0xFF);
        if (skill != 0)
        {
            dest12[1] |= 0x80;
        }

        if (luck != 0)
        {
            dest12[1] |= 0x04;
        }

        dest12[1] |= (byte)(option & 0x03);

        dest12[2] = (byte)Math.Clamp(durability, 0, 255);

        dest12[3] = 0;
        dest12[3] |= (byte)(((index & 256) >> 1) & 0xFF);
        if (option > 3)
        {
            dest12[3] |= 0x40;
        }

        dest12[3] |= (byte)(excOpt & 0xFF);

        dest12[4] = (byte)(anc & 0xFF);

        dest12[5] = 0;
        dest12[5] |= (byte)(((index & 0x1E00) >> 5) & 0xFF);
        dest12[5] |= (byte)(((oex & 128) >> 4) & 0xFF);

        dest12[6] = (byte)(joh & 0xFF);

        dest12[7] = (byte)Math.Clamp(socket1, 0, 255);
        dest12[8] = (byte)Math.Clamp(socket2, 0, 255);
        dest12[9] = (byte)Math.Clamp(socket3, 0, 255);
        dest12[10] = (byte)Math.Clamp(socket4, 0, 255);
        dest12[11] = (byte)Math.Clamp(socket5, 0, 255);
    }
}
