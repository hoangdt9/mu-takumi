namespace Takumi.Server.Protocol;

/// <summary>
/// Builds the 17-byte equipment preview for <see cref="CharacterListWire602"/> from Season 6 wear-slot item blobs.
/// Packing matches legacy <c>ObjectManager.cpp</c> <c>CharacterMakePreviewCharSet</c> / <c>ChangeCharacterExt</c>.
/// </summary>
public static class CharacterListEquipPreview602
{
    public const int PreviewLength = 17;

    private const int MaxItemType = 512;

    /// <summary>Maps wear slots 0–11 (12-byte wire each) into the 17-byte roster preview.</summary>
    public static byte[] BuildFromWearItems(ReadOnlySpan<byte[]?> wearBySlot)
    {
        var preview = new byte[PreviewLength];
        for (var i = 0; i < PreviewLength; i++)
        {
            preview[i] = 0xFF;
        }

        if (wearBySlot.Length < 9)
        {
            return preview;
        }

        var temp = new int[9];
        for (var i = 0; i < 9; i++)
        {
            var item = i < wearBySlot.Length ? wearBySlot[i] : null;
            temp[i] = DecodeTempIndex(item, i);
        }

        var charSet = new byte[18];
        charSet[1] = (byte)(temp[0] % 256);
        charSet[12] |= (byte)((temp[0] / 16) & 0xF0);

        if (temp[1] != 0xFFFF)
        {
            charSet[2] = (byte)(temp[1] % 256);
            charSet[13] |= (byte)((temp[1] / 16) & 0xF0);
        }

        charSet[3] |= (byte)((temp[2] & 0x0F) << 4);
        charSet[9] |= (byte)((temp[2] & 0x10) << 3);
        charSet[13] |= (byte)((temp[2] & 0x1E0) >> 5);

        charSet[3] |= (byte)(temp[3] & 0x0F);
        charSet[9] |= (byte)((temp[3] & 0x10) << 2);
        charSet[14] |= (byte)((temp[3] & 0x1E0) >> 1);

        charSet[4] |= (byte)((temp[4] & 0x0F) << 4);
        charSet[9] |= (byte)((temp[4] & 0x10) << 1);
        charSet[14] |= (byte)((temp[4] & 0x1E0) >> 5);

        charSet[4] |= (byte)(temp[5] & 0x0F);
        charSet[9] |= (byte)((temp[5] & 0x10));
        charSet[15] |= (byte)((temp[5] & 0x1E0) >> 1);

        charSet[5] |= (byte)((temp[6] & 0x0F) << 4);
        charSet[9] |= (byte)((temp[6] & 0x10) >> 1);
        charSet[15] |= (byte)((temp[6] & 0x1E0) >> 5);

        PackLevelsAndExcellent(charSet, wearBySlot);
        PackWingAndHelper(charSet, temp[7], temp[8]);

        for (var i = 0; i < PreviewLength; i++)
        {
            preview[i] = charSet[i + 1];
        }

        return preview;
    }

    /// <summary>Legacy <c>CharacterMakePreviewCharSet</c> level + excellent bits (slots 0–6).</summary>
    static void PackLevelsAndExcellent(byte[] charSet, ReadOnlySpan<byte[]?> wearBySlot)
    {
        ReadOnlySpan<byte> table = stackalloc byte[] { 1, 0, 6, 5, 4, 3, 2 };
        var levelBits = 0;
        for (var n = 0; n < 7; n++)
        {
            var item = n < wearBySlot.Length ? wearBySlot[n] : null;
            if (!IsWearItemPresent(item, n))
            {
                continue;
            }

            var itemLevel = ItemWire602.DecodeLevel(item!);
            if (itemLevel > 0)
            {
                levelBits |= ((itemLevel - 1) / 2) << (n * 3);
            }

            var excellent = item!.Length >= 4 ? item[3] & 0x3F : 0;
            if (excellent != 0)
            {
                charSet[10] |= (byte)(2 << table[n]);
            }
        }

        charSet[6] = (byte)(levelBits >> 16);
        charSet[7] = (byte)(levelBits >> 8);
        charSet[8] = (byte)levelBits;
    }

    /// <summary>
    /// Wear slots 7 (wing) and 8 (helper) → CharSet[5]/[9]/[10]/[12]/[16]/[17].
    /// Empty helper must set CharSet[5]|=3 (client Equipment[4]&amp;3==3 → no Guardian Angel).
    /// Empty wing must not set CharSet[5]|=12 (false fly tier / ghost cape).
    /// </summary>
    static void PackWingAndHelper(byte[] charSet, int wingTemp, int helperTemp)
    {
        // Wing nibble lives in CharSet[9] bits 0–2; clear before OR so armor bit-packing cannot shift DK nibble (2).
        charSet[9] &= 0xF8;

        if (wingTemp is >= 0 and <= 2)
        {
            charSet[5] |= 4;
            charSet[9] |= (byte)(wingTemp + 1);
        }
        else if (wingTemp is >= 3 and <= 6)
        {
            charSet[5] |= 8;
            charSet[9] |= (byte)(wingTemp - 2);
        }
        else if (wingTemp == 30)
        {
            charSet[5] |= 8;
            charSet[9] |= 5;
        }
        else if (wingTemp is >= 36 and <= 40)
        {
            charSet[5] |= 12;
            charSet[9] |= (byte)(wingTemp - 35);
        }
        else if (wingTemp == 41)
        {
            charSet[5] |= 4;
            charSet[9] |= 4;
        }
        else if (wingTemp == 42)
        {
            charSet[5] |= 8;
            charSet[9] |= 6;
        }
        else if (wingTemp == 43)
        {
            charSet[5] |= 12;
            charSet[9] |= 6;
        }
        else if (wingTemp == 49)
        {
            charSet[5] |= 8;
            charSet[9] |= 7;
        }
        else if (wingTemp == 50)
        {
            charSet[5] |= 12;
            charSet[9] |= 7;
        }

        // wingTemp == 0x1FF → leave wing bits clear (never CharSet[5]|=12 on empty wing).

        if (helperTemp == 0x1FF)
        {
            charSet[5] |= 3;
        }
        else if (helperTemp is >= 0 and <= 2)
        {
            charSet[5] |= (byte)helperTemp;
        }
        else if (helperTemp == 3)
        {
            charSet[5] |= 3;
            charSet[10] |= 1;
        }
        else if (helperTemp == 4)
        {
            charSet[5] |= 3;
            charSet[12] |= 1;
        }
        else if (helperTemp == 37)
        {
            charSet[5] |= 3;
            charSet[10] &= 0xFE;
            charSet[12] &= 0xFE;
            charSet[12] |= 4;
        }
        else if (helperTemp is 64 or 65 or 67)
        {
            charSet[16] |= (byte)((helperTemp - 63) << 5);
        }
        else if (helperTemp == 80)
        {
            charSet[16] |= 0xE0;
        }
        else if (helperTemp == 106)
        {
            charSet[16] |= 0xA0;
        }
        else if (helperTemp == 123)
        {
            charSet[16] |= 0x60;
        }
    }

    static bool IsWearItemPresent(byte[]? item12, int wearSlot)
    {
        if (item12 is null || item12.Length < ItemWire602.WireBytes || ItemWire602.IsEmpty(item12))
        {
            return false;
        }

        var isWeapon = wearSlot is 0 or 1;
        if (item12[0] == 0xFF && (item12[3] & 0x80) == 0x80 && (item12[5] & 0xF0) == 0xF0)
        {
            return false;
        }

        var temp = DecodeTempIndex(item12, wearSlot);
        return isWeapon ? temp != 0xFFFF : temp != 0x1FF;
    }

    static int DecodeTempIndex(byte[]? item12, int wearSlot)
    {
        var isWeapon = wearSlot is 0 or 1;
        if (item12 is null || item12.Length < ItemWire602.WireBytes || ItemWire602.IsEmpty(item12))
        {
            return isWeapon ? 0xFFFF : 0x1FF;
        }

        if (item12[0] == 0xFF && (item12[3] & 0x80) == 0x80 && (item12[5] & 0xF0) == 0xF0)
        {
            return isWeapon ? 0xFFFF : 0x1FF;
        }

        var index = ItemWire602.DecodeItemIndex(item12);
        return isWeapon ? index : index % MaxItemType;
    }
}
