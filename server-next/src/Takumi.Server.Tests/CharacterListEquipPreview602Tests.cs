using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CharacterListEquipPreview602Tests
{
    static byte[] WearItem(int group, int index, int level = 15, int excellent = 63)
    {
        var blob = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(blob, group, index, level, 255, true, true, 7, excellent);
        return blob;
    }

    [Fact]
    public void BuildFromWearItems_MgSaintSet_EncodesArmorType142()
    {
        var wear = new byte[]?[12];
        wear[0] = WearItem(0, 58);
        wear[3] = WearItem(8, 142);
        wear[4] = WearItem(9, 142);
        wear[5] = WearItem(10, 142);
        wear[6] = WearItem(11, 142);

        var preview = CharacterListEquipPreview602.BuildFromWearItems(wear);

        Assert.Equal(17, preview.Length);
        Assert.NotEqual(0xFF, preview[2]); // armor nibble in Equipment[3] low

        var armorExt = (preview[2] & 0x0F) + ((preview[8] >> 6) & 1) * 16 + ((preview[13] >> 4) & 15) * 32;
        Assert.Equal(142, armorExt);

        var levelWord = (preview[5] << 16) + (preview[6] << 8) + preview[7];
        Assert.Equal(7, (levelWord >> 9) & 7); // +15 armor → legacy (15-1)/2
    }

    [Fact]
    public void BuildFromWearItems_MgSaintSet_WithStormWing_EncodesWingTier3()
    {
        var wear = new byte[]?[12];
        wear[0] = WearItem(0, 58);
        wear[7] = WearItem(12, 36);
        wear[3] = WearItem(8, 142);
        wear[4] = WearItem(9, 142);
        wear[5] = WearItem(10, 142);
        wear[6] = WearItem(11, 142);

        var preview = CharacterListEquipPreview602.BuildFromWearItems(wear);

        Assert.Equal(12, preview[4] & 0x0C);
    }

    [Fact]
    public void BuildFromWearItems_MgSaintSet_NoWing_DoesNotSetFalseWingBitsOnEquipment4()
    {
        var wear = new byte[]?[12];
        wear[0] = WearItem(0, 58);
        wear[3] = WearItem(8, 142);
        wear[4] = WearItem(9, 142);
        wear[5] = WearItem(10, 142);
        wear[6] = WearItem(11, 142);

        var preview = CharacterListEquipPreview602.BuildFromWearItems(wear);

        // preview[4] == CharSet[5]: wing tier is (Equipment[4]>>2)&3 — must stay 0 when no wing equipped.
        Assert.Equal(0, preview[4] & 0x0C);
        // Empty helper → legacy CharSet[5]|=3 so client does not spawn Guardian Angel (Equipment[4]&3==0).
        Assert.Equal(3, preview[4] & 0x03);
    }

    [Fact]
    public void BuildFromWearItems_EmptyWear_SetsHelperAbsentMarker()
    {
        var wear = new byte[]?[12];
        var preview = CharacterListEquipPreview602.BuildFromWearItems(wear);
        Assert.Equal(3, preview[4] & 0x03);
        Assert.Equal(0, preview[4] & 0x0C);
    }

    [Fact]
    public void BuildFromWearItems_MgSeedHex_WeaponPreviewDecodesToMgSaintSword58()
    {
        var wear = new byte[]?[12];
        wear[0] = Convert.FromHexString("3AFFFF7F0000000000000000");
        wear[7] = Convert.FromHexString("27FFFF7F00C0000000000000");
        wear[3] = Convert.FromHexString("8EFFFF7F0080000000000000");
        wear[4] = Convert.FromHexString("8EFFFF7F0090000000000000");
        wear[5] = Convert.FromHexString("8EFFFF7F00A0000000000000");
        wear[6] = Convert.FromHexString("8EFFFF7F00B0000000000000");

        var preview = CharacterListEquipPreview602.BuildFromWearItems(wear);

        var extType = (preview[11] & 240) << 4 | preview[0];
        Assert.Equal(58, extType);
    }

    [Fact]
    public void BuildFromWearItems_MgSeedHex_WeaponPreviewDecodesLikeClient()
    {
        var wear = new byte[]?[12];
        wear[0] = Convert.FromHexString("3AFFFF7F0000000000000000");
        wear[7] = Convert.FromHexString("27FFFF7F00C0000000000000");
        wear[3] = Convert.FromHexString("8EFFFF7F0080000000000000");
        wear[4] = Convert.FromHexString("8EFFFF7F0090000000000000");
        wear[5] = Convert.FromHexString("8EFFFF7F00A0000000000000");
        wear[6] = Convert.FromHexString("8EFFFF7F00B0000000000000");

        var preview = CharacterListEquipPreview602.BuildFromWearItems(wear);

        var high = preview[11] & 0xF0;
        if (high == 0 && (preview[12] & 0xF0) != 0)
        {
            high = (byte)(preview[12] & 0xF0);
        }

        var extType = (high << 4) | preview[0];
        Assert.Equal(58, extType);
    }

    [Fact]
    public void BuildFromWearItems_MgSeedHex_WingRosterNibbleIsLoiVu39()
    {
        var wear = new byte[]?[12];
        wear[0] = Convert.FromHexString("3AFFFF7F0000000000000000");
        wear[7] = Convert.FromHexString("27FFFF7F00C0000000000000");
        wear[3] = Convert.FromHexString("8EFFFF7F0080000000000000");
        wear[4] = Convert.FromHexString("8EFFFF7F0090000000000000");
        wear[5] = Convert.FromHexString("8EFFFF7F00A0000000000000");
        wear[6] = Convert.FromHexString("8EFFFF7F00B0000000000000");

        var preview = CharacterListEquipPreview602.BuildFromWearItems(wear);

        Assert.Equal(4, preview[8] & 0x07);
        Assert.NotEqual(1, preview[8] & 0x07);
        Assert.Equal(0, (preview[15] >> 2) & 0x07);
    }

    [Fact]
    public void DecodeItemIndex_MgSeedHex_UsesSaintMgNotHadesSm()
    {
        var mgSaintArmor = Convert.FromHexString("8EFFFF7F0080000000000000");
        var smHadesArmor = Convert.FromHexString("1EFFFF7F0080000000000000");
        var mgSaintSword = Convert.FromHexString("3AFFFF7F0000000000000000");
        var sumKhuyen = Convert.FromHexString("36FFFF7F0050000000000000");

        Assert.Equal(142, ItemWire602.DecodeItemIndex(mgSaintArmor) % 512);
        Assert.Equal(30, ItemWire602.DecodeItemIndex(smHadesArmor) % 512);
        Assert.Equal(58, ItemWire602.DecodeItemIndex(mgSaintSword) % 512);
        Assert.Equal(54, ItemWire602.DecodeItemIndex(sumKhuyen) % 512);
    }
}
