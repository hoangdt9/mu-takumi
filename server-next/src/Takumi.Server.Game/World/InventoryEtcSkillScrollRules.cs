namespace Takumi.Server.Game.World;

/// <summary>
/// Skill scrolls in item <b>group 15</b> (<c>ITEM_ETC</c> in Main 5.2), numbers 0–36 — parity OpenMU Season6
/// <see href="https://github.com/MUnique/OpenMU/blob/master/src/Persistence/Initialization/VersionSeasonSix/Items/Scrolls.cs">Scrolls.cs</see>.
/// </summary>
public static class InventoryEtcSkillScrollRules
{
    const int EtcGroupBase = 15 * 512;

    static uint ClassMask(int dw, int dk, int fe, int mg, int dl, int su, int rf)
    {
        uint m = 0;
        if (dw != 0)
        {
            m |= 1u << 0;
        }

        if (dk != 0)
        {
            m |= 1u << 1;
        }

        if (fe != 0)
        {
            m |= 1u << 2;
        }

        if (mg != 0)
        {
            m |= 1u << 3;
        }

        if (dl != 0)
        {
            m |= 1u << 4;
        }

        if (su != 0)
        {
            m |= 1u << 5;
        }

        if (rf != 0)
        {
            m |= 1u << 6;
        }

        return m;
    }

    static byte MinLevel(byte dropLevel, int levelRequirement)
    {
        var v = Math.Max((int)dropLevel, levelRequirement);
        return (byte)Math.Clamp(v < 1 ? 1 : v, 1, 255);
    }

    /// <summary>Maps full item index (group×512 + number) to learn rule.</summary>
    static readonly Dictionary<int, InventorySkillOrbRules.SkillOrbLearn> ScrollByIndex = new()
    {
        // Scroll(number), skillNumber, dropLevel, levelReq, energyReq, DW, DK, ELF, MG, DL, SU, RF
        [EtcGroupBase + 0] = new(1, MinLevel(30, 0), ClassMask(1, 0, 0, 1, 0, 0, 0), 140),
        [EtcGroupBase + 1] = new(2, MinLevel(21, 0), ClassMask(1, 0, 0, 1, 0, 1, 0), 104),
        [EtcGroupBase + 2] = new(3, MinLevel(13, 0), ClassMask(1, 0, 0, 1, 0, 0, 0), 72),
        [EtcGroupBase + 3] = new(4, MinLevel(5, 0), ClassMask(1, 0, 0, 1, 0, 1, 0), 40),
        [EtcGroupBase + 4] = new(5, MinLevel(35, 0), ClassMask(1, 0, 0, 1, 0, 0, 0), 160),
        [EtcGroupBase + 5] = new(6, MinLevel(17, 0), ClassMask(1, 0, 0, 0, 0, 0, 0), 88),
        [EtcGroupBase + 6] = new(7, MinLevel(25, 0), ClassMask(1, 0, 0, 1, 0, 1, 0), 120),
        [EtcGroupBase + 7] = new(8, MinLevel(40, 0), ClassMask(1, 0, 0, 1, 0, 0, 0), 180),
        [EtcGroupBase + 8] = new(9, MinLevel(50, 0), ClassMask(1, 0, 0, 1, 0, 0, 0), 220),
        [EtcGroupBase + 9] = new(10, MinLevel(60, 0), ClassMask(1, 0, 0, 1, 0, 0, 0), 260),
        [EtcGroupBase + 10] = new(11, MinLevel(9, 0), ClassMask(1, 0, 0, 1, 0, 1, 0), 56),
        [EtcGroupBase + 11] = new(12, MinLevel(74, 0), ClassMask(1, 0, 0, 1, 0, 0, 0), 345),
        [EtcGroupBase + 12] = new(13, MinLevel(80, 0), ClassMask(1, 0, 0, 1, 0, 0, 0), 436),
        [EtcGroupBase + 13] = new(14, MinLevel(88, 0), ClassMask(1, 0, 0, 1, 0, 0, 0), 578),
        [EtcGroupBase + 14] = new(15, MinLevel(83, 0), ClassMask(2, 0, 0, 0, 0, 0, 0), 644),
        [EtcGroupBase + 15] = new(16, MinLevel(77, 0), ClassMask(1, 0, 0, 0, 0, 0, 0), 408),
        [EtcGroupBase + 16] = new(38, MinLevel(96, 0), ClassMask(2, 0, 0, 0, 0, 0, 0), 953),
        [EtcGroupBase + 17] = new(39, MinLevel(93, 0), ClassMask(2, 0, 0, 0, 0, 0, 0), 849),
        [EtcGroupBase + 18] = new(40, MinLevel(100, 0), ClassMask(2, 0, 0, 0, 0, 0, 0), 1052),
        [EtcGroupBase + 19] = new(215, MinLevel(75, 0), ClassMask(0, 0, 0, 0, 0, 1, 0), 245),
        [EtcGroupBase + 20] = new(214, MinLevel(35, 0), ClassMask(0, 0, 0, 0, 0, 1, 0), 150),
        [EtcGroupBase + 21] = new(230, MinLevel(93, 0), ClassMask(0, 0, 0, 0, 0, 1, 0), 823),
        [EtcGroupBase + 22] = new(217, MinLevel(80, 0), ClassMask(0, 0, 0, 0, 0, 1, 0), 375),
        [EtcGroupBase + 23] = new(218, MinLevel(83, 0), ClassMask(0, 0, 0, 0, 0, 1, 0), 620),
        [EtcGroupBase + 24] = new(219, MinLevel(40, 0), ClassMask(0, 0, 0, 0, 0, 1, 0), 180),
        [EtcGroupBase + 26] = new(221, MinLevel(93, 0), ClassMask(0, 0, 0, 0, 0, 2, 0), 663),
        [EtcGroupBase + 27] = new(222, MinLevel(111, 0), ClassMask(0, 0, 0, 0, 0, 2, 0), 912),
        [EtcGroupBase + 28] = new(233, MinLevel(100, 220), ClassMask(2, 0, 0, 0, 0, 0, 0), 118),
        [EtcGroupBase + 29] = new(237, MinLevel(100, 220), ClassMask(0, 0, 0, 1, 0, 0, 0), 118),
        [EtcGroupBase + 30] = new(262, MinLevel(80, 150), ClassMask(0, 0, 0, 0, 0, 0, 1), 0),
        [EtcGroupBase + 31] = new(263, MinLevel(100, 180), ClassMask(0, 0, 0, 0, 0, 0, 1), 0),
        [EtcGroupBase + 32] = new(264, MinLevel(90, 150), ClassMask(0, 0, 0, 0, 0, 0, 1), 0),
        [EtcGroupBase + 33] = new(265, MinLevel(100, 200), ClassMask(0, 0, 0, 0, 0, 0, 1), 0),
        [EtcGroupBase + 34] = new(266, MinLevel(100, 120), ClassMask(0, 0, 0, 0, 0, 0, 1), 404),
        [EtcGroupBase + 35] = new(267, MinLevel(90, 80), ClassMask(0, 0, 0, 0, 0, 0, 1), 132),
        [EtcGroupBase + 36] = new(268, MinLevel(70, 50), ClassMask(0, 0, 0, 0, 0, 0, 1), 80),
    };

    public static bool TryGetSkillScrollLearn(int itemIndex, out InventorySkillOrbRules.SkillOrbLearn learn) =>
        ScrollByIndex.TryGetValue(itemIndex, out learn);
}
