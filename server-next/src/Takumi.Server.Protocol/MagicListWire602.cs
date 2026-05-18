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

    public static byte[] BuildFromRows(IReadOnlyList<CharacterSkillRowLike> rows, byte listType = ListTypeNormal)
    {
        if (rows.Count == 0)
        {
            return BuildEmpty(listType);
        }

        var entries = new Entry[rows.Count];
        for (var i = 0; i < rows.Count; i++)
        {
            var row = rows[i];
            entries[i] = new Entry(row.Slot, row.Type, row.Level);
        }

        return Build(listType, entries);
    }

    public interface CharacterSkillRowLike
    {
        byte Slot { get; }

        ushort Type { get; }

        byte Level { get; }
    }

    /// <summary>MG / Duel Master QA kit: all combat skills with MG column in GameServer <c>Skill.txt</c>.</summary>
    public static byte[] BuildMagicGladiatorFull(byte maxLevel = 20) =>
        BuildForServerClass(0x60, maxLevel);

    public static byte[] BuildForServerClass(byte serverClass, byte maxLevel = 20)
    {
        var defaults = CharacterSkillCatalog.GetDefaultEntries(serverClass, maxLevel);
        if (defaults.Count == 0)
        {
            return BuildEmpty();
        }

        Span<Entry> scratch = stackalloc Entry[defaults.Count];
        for (var i = 0; i < defaults.Count; i++)
        {
            scratch[i] = defaults[i];
        }

        return Build(ListTypeNormal, scratch);
    }
}
