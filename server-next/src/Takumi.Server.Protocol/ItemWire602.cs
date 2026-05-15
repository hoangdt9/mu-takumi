namespace Takumi.Server.Protocol;

/// <summary>Season 6 item blob encoder (parity <c>CItemManager::ItemByteConvert</c>, 12 bytes).</summary>
public static class ItemWire602
{
    public const int WireBytes = 12;

    public const byte FirstBagSlot = 12;

    public const byte LastBagSlot = 75;

    public static bool IsEmpty(ReadOnlySpan<byte> item12) =>
        item12.Length < WireBytes || (item12[0] == 0 && item12[1] == 0) || item12[0] == 0xFF;

    public static void WriteSeason6Item(
        Span<byte> dest,
        int itemGroup,
        int itemIndex,
        int level,
        int durability,
        bool skill,
        bool luck,
        int option,
        int excellent)
    {
        if (dest.Length < WireBytes)
        {
            throw new ArgumentException("Destination must be at least 12 bytes.", nameof(dest));
        }

        var index = (itemGroup * 512) + itemIndex;
        dest[0] = (byte)(index & 0xFF);
        dest[1] = 0;
        dest[1] |= (byte)((level & 0x0F) << 3);
        if (skill)
        {
            dest[1] |= 128;
        }

        if (luck)
        {
            dest[1] |= 4;
        }

        dest[1] |= (byte)(option & 3);
        dest[2] = (byte)Math.Clamp(durability, 0, 255);
        dest[3] = 0;
        dest[3] |= (byte)((index & 256) >> 1);
        if (option > 3)
        {
            dest[3] |= 64;
        }

        dest[3] |= (byte)(excellent & 0x3F);
        dest[4] = 0;
        dest[5] = 0;
        dest[5] |= (byte)((index & 0x1E00) >> 5);
        dest[6] = 0;
        dest[7] = 0;
        dest[8] = 0;
        dest[9] = 0;
        dest[10] = 0;
        dest[11] = 0;
    }

    public static void SetDurability(Span<byte> item12, byte durability)
    {
        if (item12.Length >= WireBytes)
        {
            item12[2] = durability;
        }
    }
}
