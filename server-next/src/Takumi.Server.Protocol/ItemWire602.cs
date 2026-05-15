namespace Takumi.Server.Protocol;

/// <summary>Season 6 item blob encoder (parity <c>CItemManager::ItemByteConvert</c>, 12 bytes).</summary>
public static class ItemWire602
{
    public const int WireBytes = 12;

    public const byte FirstWearSlot = 0;

    public const byte LastWearSlot = 11;

    public const byte FirstBagSlot = 12;

    public const byte LastBagSlot = 75;

    public const int ZenItemIndex = (14 * 512) + 15;

    public static bool IsEmpty(ReadOnlySpan<byte> item12)
    {
        if (item12.Length < WireBytes || item12[0] == 0xFF)
        {
            return true;
        }

        for (var i = 0; i < WireBytes; i++)
        {
            if (item12[i] != 0)
            {
                return false;
            }
        }

        return true;
    }

    public static int DecodeItemIndex(ReadOnlySpan<byte> item12)
    {
        if (item12.Length < WireBytes)
        {
            return -1;
        }

        return item12[0]
               + ((item12[3] & 0x80) << 1)
               + ((item12[5] & 0xF0) << 5);
    }

    public static int DecodeLevel(ReadOnlySpan<byte> item12) =>
        item12.Length >= 2 ? (item12[1] >> 3) & 0x0F : 0;

    public static byte DecodeDurability(ReadOnlySpan<byte> item12) =>
        item12.Length >= 3 ? item12[2] : (byte)0;

    public static bool IsZenItem(ReadOnlySpan<byte> item12) => DecodeItemIndex(item12) == ZenItemIndex;

    public static bool IsWearSlot(byte slot) => slot <= LastWearSlot;

    public static bool IsBagSlot(byte slot) => slot >= FirstBagSlot && slot <= LastBagSlot;

    public static bool ItemsStackable(ReadOnlySpan<byte> a, ReadOnlySpan<byte> b) =>
        !IsEmpty(a)
        && !IsEmpty(b)
        && DecodeItemIndex(a) == DecodeItemIndex(b)
        && DecodeLevel(a) == DecodeLevel(b)
        && !IsZenItem(a);

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

    /// <summary>Encode total zen for <c>GET_ITEM_ZEN</c> pick response (<c>PRECEIVE_GET_ITEM</c>).</summary>
    public static void WriteZenPickTotal(Span<byte> dest, long zen)
    {
        if (dest.Length < 4)
        {
            return;
        }

        dest.Clear();
        var z = (uint)Math.Clamp(zen, 0, uint.MaxValue);
        dest[0] = (byte)((z >> 24) & 0xFF);
        dest[1] = (byte)((z >> 16) & 0xFF);
        dest[2] = (byte)((z >> 8) & 0xFF);
        dest[3] = (byte)(z & 0xFF);
    }
}
