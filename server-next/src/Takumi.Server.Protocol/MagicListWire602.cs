namespace Takumi.Server.Protocol;

/// <summary>Season 6 skill list (<c>C1 F3 11</c> / <c>GCSkillListSend</c>).</summary>
public static class MagicListWire602
{
    public const byte ListTypeMaster = 0;
    public const byte ListTypeNormal = 1;

    public readonly record struct Entry(byte Index, ushort Type, byte Level);

    /// <summary>Empty skill list (client accepts count=0).</summary>
    public static byte[] BuildEmpty(byte listType = ListTypeNormal) => Build(listType, ReadOnlySpan<Entry>.Empty);

    public static byte[] Build(byte listType, ReadOnlySpan<Entry> skills)
    {
        var size = 6 + (skills.Length * 4);
        var buf = new byte[size];
        buf[0] = 0xC1;
        buf[1] = (byte)size;
        buf[2] = 0xF3;
        buf[3] = 0x11;
        buf[4] = listType;
        buf[5] = (byte)skills.Length;

        var offset = 6;
        foreach (var skill in skills)
        {
            buf[offset++] = skill.Index;
            buf[offset++] = (byte)(skill.Type & 0xFF);
            buf[offset++] = (byte)((skill.Type >> 8) & 0xFF);
            buf[offset++] = skill.Level;
        }

        return buf;
    }

    /// <summary>MG / Duel Master QA kit: all combat skills with MG column in GameServer <c>Skill.txt</c>.</summary>
    public static byte[] BuildMagicGladiatorFull(byte maxLevel = 20)
    {
        ReadOnlySpan<ushort> skillIds =
        [
            1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 17,
            18, 19, 20, 21, 22, 23,
            41, 47,
            55, 56, 57, 73,
            236, 237,
        ];

        var entries = new Entry[skillIds.Length];
        for (var i = 0; i < skillIds.Length; i++)
        {
            var id = skillIds[i];
            entries[i] = new Entry((byte)id, id, maxLevel);
        }

        return Build(ListTypeNormal, entries);
    }

    public static byte[] BuildForServerClass(byte serverClass, byte maxLevel = 20) =>
        CharacterSheetCalculator.ClassIndex(serverClass) == 3
            ? BuildMagicGladiatorFull(maxLevel)
            : BuildEmpty();
}
