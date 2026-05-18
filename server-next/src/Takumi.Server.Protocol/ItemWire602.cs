namespace Takumi.Server.Protocol;

/// <summary>Season 6 item blob encoder (parity <c>CItemManager::ItemByteConvert</c>, 12 bytes).</summary>
public static class ItemWire602
{
    public const int WireBytes = 12;

    /// <summary>Unused socket slot (parity OpenMU / legacy <c>0xFF</c>).</summary>
    public const byte NoSocket = 0xFF;

    /// <summary>Empty socket slot (seeded, no option yet).</summary>
    public const byte EmptySocket = 0xFE;

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

    public static int DecodeExcellentOptions(ReadOnlySpan<byte> item12) =>
        item12.Length >= 4 ? item12[3] & 0x3F : 0;

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
        WriteSeason6Core(dest, itemGroup, itemIndex, level, durability, skill, luck, option, excellent);
        dest[6] = 0;
        dest.Slice(7, 5).Fill(NoSocket);
    }

    /// <summary>
    /// Shop / NPC stock encoding (parity <c>CShop::InsertItemNew</c> + <c>ItemByteConvert</c>).
    /// Socket columns apply only when <paramref name="isSocketItem"/> is true.
    /// </summary>
    public static void WriteShopItem(
        Span<byte> dest,
        int itemGroup,
        int itemIndex,
        int level,
        int durability,
        bool skill,
        bool luck,
        int option,
        int excellent,
        int ancient,
        int jewelOfHarmony,
        int itemOptionEx,
        bool isSocketItem,
        int maxSocket,
        int socket1,
        int socket2,
        int socket3,
        int socket4,
        int socket5,
        byte socketBonus = NoSocket)
    {
        WriteSeason6Core(dest, itemGroup, itemIndex, level, durability, skill, luck, option, excellent);
        var index = (itemGroup * 512) + itemIndex;
        dest[4] = (byte)Math.Clamp(ancient, 0, 255);
        dest[5] = 0;
        dest[5] |= (byte)(((index & 0x1E00) >> 5) & 0xFF);
        dest[5] |= (byte)(((itemOptionEx & 128) >> 4) & 0xFF);

        if (isSocketItem && maxSocket > 0)
        {
            dest[6] = socketBonus;
            WriteSocketColumns(dest.Slice(7, 5), maxSocket, socket1, socket2, socket3, socket4, socket5);
            return;
        }

        dest[6] = (byte)Math.Clamp(jewelOfHarmony, 0, 255);
        dest.Slice(7, 5).Fill(NoSocket);
    }

    public static void WriteShopItem(Span<byte> dest, ShopItemWireSource source)
    {
        WriteShopItem(
            dest,
            source.ItemGroup,
            source.ItemIndex,
            source.Level,
            source.Durability,
            source.Skill,
            source.Luck,
            source.Option,
            source.Excellent,
            source.Ancient,
            source.JewelOfHarmony,
            source.ItemOptionEx,
            source.IsSocketItem,
            source.MaxSocket,
            source.Socket1,
            source.Socket2,
            source.Socket3,
            source.Socket4,
            source.Socket5);
    }

    public readonly record struct ShopItemWireSource(
        int ItemGroup,
        int ItemIndex,
        int Level,
        int Durability,
        bool Skill,
        bool Luck,
        int Option,
        int Excellent,
        int Ancient,
        int JewelOfHarmony,
        int ItemOptionEx,
        bool IsSocketItem,
        int MaxSocket,
        int Socket1,
        int Socket2,
        int Socket3,
        int Socket4,
        int Socket5);

    static void WriteSeason6Core(
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
    }

    /// <summary>Parity <c>CShop::InsertItemNew</c> socket column gating by max socket count.</summary>
    public static void WriteSocketColumns(
        Span<byte> dest,
        int maxSocket,
        int socket1,
        int socket2,
        int socket3,
        int socket4,
        int socket5)
    {
        ReadOnlySpan<int> sockets = [socket1, socket2, socket3, socket4, socket5];
        for (var i = 0; i < sockets.Length; i++)
        {
            if (i >= maxSocket)
            {
                dest[i] = NoSocket;
                continue;
            }

            var v = sockets[i];
            dest[i] = (byte)Math.Clamp(v, 0, 255);
        }
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
