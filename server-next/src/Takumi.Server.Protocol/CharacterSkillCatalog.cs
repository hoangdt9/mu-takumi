namespace Takumi.Server.Protocol;

/// <summary>Default learned skills per roster class when <c>character_skill</c> has no rows yet.</summary>
public static class CharacterSkillCatalog
{
    public static IReadOnlyList<MagicListWire602.Entry> GetDefaultEntries(byte serverClass, byte maxLevel = 20)
    {
        return CharacterSheetCalculator.ClassIndex(serverClass) switch
        {
            3 => ToEntries(MagicGladiatorSkillTypes, maxLevel),
            2 => ToEntries(ElfSkillTypes, maxLevel),
            1 => ToEntries(DarkKnightSkillTypes, maxLevel),
            4 => ToEntries(DarkLordSkillTypes, maxLevel),
            5 => ToEntries(SummonerSkillTypes, maxLevel),
            6 => ToEntries(RageFighterSkillTypes, maxLevel),
            _ => ToEntries(DarkWizardSkillTypes, maxLevel),
        };
    }

    static readonly ushort[] DarkWizardSkillTypes =
    [
        3, 4, 5, 6, 7, 8, 12, 13, 14, 17,
    ];

    static readonly ushort[] DarkKnightSkillTypes =
    [
        19, 20, 21, 22, 23, 41, 42, 43, 44,
    ];

    /// <summary>Includes poison (Độc) for hotkey QA on Elf characters.</summary>
    static readonly ushort[] ElfSkillTypes =
    [
        1, 3, 24, 25, 26, 27, 28, 52,
    ];

    static readonly ushort[] MagicGladiatorSkillTypes =
    [
        1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 17,
        18, 19, 20, 21, 22, 23,
        41, 47,
        55, 56, 57, 73,
        236, 237,
    ];

    static readonly ushort[] DarkLordSkillTypes =
    [
        49, 55, 60, 61, 62, 63,
    ];

    static readonly ushort[] SummonerSkillTypes =
    [
        214, 215, 216, 217, 218, 219, 220, 221, 222,
    ];

    static readonly ushort[] RageFighterSkillTypes =
    [
        260, 261, 262, 263, 264, 265, 266, 267,
    ];

    static MagicListWire602.Entry[] ToEntries(ushort[] types, byte maxLevel)
    {
        var entries = new MagicListWire602.Entry[types.Length];
        for (var i = 0; i < types.Length; i++)
        {
            var id = types[i];
            var slot = id <= byte.MaxValue ? (byte)id : (byte)Math.Min(i, byte.MaxValue);
            entries[i] = new MagicListWire602.Entry(slot, id, maxLevel);
        }

        return entries;
    }
}
