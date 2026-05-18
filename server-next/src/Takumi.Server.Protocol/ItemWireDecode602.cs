namespace Takumi.Server.Protocol;

/// <summary>Decode Season 6 item wire fields (parity <c>CItemManager::ItemByteConvert</c>).</summary>
public static class ItemWireDecode602
{
    public static int DecodeOption1Skill(ReadOnlySpan<byte> item12) =>
        item12.Length >= 2 && (item12[1] & 128) != 0 ? 1 : 0;

    public static int DecodeOption2Luck(ReadOnlySpan<byte> item12) =>
        item12.Length >= 2 && (item12[1] & 4) != 0 ? 1 : 0;

    public static int DecodeOption3(ReadOnlySpan<byte> item12)
    {
        if (item12.Length < 4)
        {
            return 0;
        }

        return (item12[1] & 3) + ((item12[3] & 64) != 0 ? 4 : 0);
    }

    public static int DecodeNewOption(ReadOnlySpan<byte> item12) =>
        item12.Length >= 4 ? item12[3] & 0x3F : 0;

    public static int DecodeSetOption(ReadOnlySpan<byte> item12) =>
        item12.Length >= 5 ? item12[4] & 0x0F : 0;

    public static bool HasAnyExcellent(int newOption) => newOption != 0;

    public static int GetItemGroup(int itemIndex) => itemIndex / 512;

    public static int GetItemNumber(int itemIndex) => itemIndex % 512;

    public static bool IsWeaponIndex(int itemIndex) =>
        itemIndex is >= 0 and < 3072;

    public static bool IsStaffIndex(int itemIndex) =>
        itemIndex is >= 2560 and < 3072;

    public static bool IsBowIndex(int itemIndex) =>
        itemIndex is >= 2048 and < 2560;

    public static bool IsShieldIndex(int itemIndex) =>
        itemIndex is >= 3072 and < 3584;

    public static bool IsArmorIndex(int itemIndex) =>
        itemIndex is >= 3584 and < 6144;

    public static bool IsWingIndex(int itemIndex) =>
        itemIndex is >= 6144 and < 6688;

    public static bool IsSocketItem(ReadOnlySpan<byte> item12)
    {
        if (item12.Length < 12)
        {
            return false;
        }

        foreach (var b in item12.Slice(7, 5))
        {
            if (b != ItemWire602.NoSocket && b != ItemWire602.EmptySocket)
            {
                return true;
            }
        }

        return false;
    }

    public static int DecodeJewelOfHarmonyOption(ReadOnlySpan<byte> item12) =>
        item12.Length >= 7 ? item12[6] : 0;
}
