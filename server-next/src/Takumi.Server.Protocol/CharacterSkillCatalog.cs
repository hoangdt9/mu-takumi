namespace Takumi.Server.Protocol;

/// <summary>Default learned skills per roster class when <c>character_skill</c> has no rows yet.</summary>
public static class CharacterSkillCatalog
{
    /// <summary>Client <c>MAX_MAGIC</c> — picker scans <c>Skill[0..ClientMagicSlotLimit)</c>.</summary>
    public const byte ClientMagicSlotLimit = 150;

    public static IReadOnlyList<MagicListWire602.Entry> GetDefaultEntries(byte serverClass, byte maxLevel = 20)
    {
        return CharacterSheetCalculator.ClassIndex(serverClass) switch
        {
            3 => ToMagicGladiatorCombatEntries(maxLevel),
            2 => ToEntries(ElfSkillTypes, maxLevel),
            1 => ToEntries(DarkKnightSkillTypes, maxLevel),
            4 => ToEntries(DarkLordSkillTypes, maxLevel),
            5 => ToEntries(SummonerSkillTypes, maxLevel),
            6 => ToEntries(RageFighterSkillTypes, maxLevel),
            _ => ToEntries(DarkWizardSkillTypes, maxLevel),
        };
    }

    /// <summary>MG combat QA: compact slots 1..N, real <see cref="MagicGladiatorCombatSkillTypes"/>.</summary>
    public static MagicListWire602.Entry[] ToMagicGladiatorCombatEntries(byte maxLevel = 20) =>
        AssignCompactSlots(MagicGladiatorCombatSkillTypes, maxLevel);

    /// <summary>
    /// Remap DB rows to client-safe slots before <c>F3 11</c>. Drops master/support extras; avoids slot=type≥150 blind spots.
    /// </summary>
    public static MagicListWire602.Entry[] NormalizeMagicGladiatorForClientWire(
        IReadOnlyList<MagicListWire602.Entry> rows,
        byte maxLevel = 20)
    {
        if (rows.Count == 0)
        {
            return [];
        }

        if (IsAlreadyCompactCombatLayout(rows))
        {
            return rows.ToArray();
        }

        var levelByType = new Dictionary<ushort, byte>();
        foreach (var row in rows)
        {
            if (row.Type == 0 || IsMasterSkill(row.Type))
            {
                continue;
            }

            levelByType[row.Type] = row.Level;
        }

        var compact = new List<MagicListWire602.Entry>(MagicGladiatorCombatSkillTypes.Length);
        byte slot = 1;
        foreach (var type in MagicGladiatorCombatSkillTypes)
        {
            if (!levelByType.TryGetValue(type, out var level))
            {
                continue;
            }

            if (slot >= ClientMagicSlotLimit)
            {
                break;
            }

            compact.Add(new MagicListWire602.Entry(slot++, type, level == 0 ? maxLevel : level));
        }

        return compact.ToArray();
    }

    static bool IsAlreadyCompactCombatLayout(IReadOnlyList<MagicListWire602.Entry> rows)
    {
        if (rows.Count != MagicGladiatorCombatSkillTypes.Length)
        {
            return false;
        }

        for (var i = 0; i < rows.Count; i++)
        {
            var expectedSlot = (byte)(i + 1);
            if (rows[i].Index != expectedSlot
                || rows[i].Type != MagicGladiatorCombatSkillTypes[i])
            {
                return false;
            }
        }

        return true;
    }

    static MagicListWire602.Entry[] AssignCompactSlots(ushort[] types, byte maxLevel)
    {
        var entries = new MagicListWire602.Entry[types.Length];
        byte slot = 1;
        for (var i = 0; i < types.Length; i++)
        {
            entries[i] = new MagicListWire602.Entry(slot++, types[i], maxLevel);
        }

        return entries;
    }

    static readonly ushort[] DarkWizardSkillTypes =
    [
        3, 4, 5, 6, 7, 8, 12, 13, 14, 17,
    ];

    static readonly ushort[] DarkKnightSkillTypes =
    [
        19, 20, 21, 22, 23, 41, 42, 43, 44,
    ];

    static readonly ushort[] ElfSkillTypes =
    [
        1, 3, 24, 25, 26, 27, 28, 52,
    ];

    /// <summary>MG combat picker QA (<c>SKILL-QA-CHECKLIST</c> MG combat rows).</summary>
    public static readonly ushort[] MagicGladiatorCombatSkillTypes =
    [
        1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 17,
        18, 19, 20, 21, 22, 23,
        39, 41, 47,
        55, 56, 57,
        73, 76,
        236, 237,
    ];

    /// <summary>Not sent on <c>F3 11</c> until master tree <c>F3 53</c> is wired.</summary>
    internal static readonly ushort[] MagicGladiatorMasterSkillTypes =
    [
        344, 346, 348, 349, 352, 353, 361, 364,
        397, 398, 400, 401, 405, 407,
        476, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497,
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

    static bool IsMasterSkill(ushort type) => Array.IndexOf(MagicGladiatorMasterSkillTypes, type) >= 0;

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
