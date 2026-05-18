using System.Text;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Join-time skill list: load DB → seed class defaults → persist → <c>F3 11</c>.</summary>
public static class JoinSkillLifecycle
{
    public static async Task<byte[]> BuildJoinPacketAsync(
        PostgresCharacterSkillRepository? skillRepo,
        string? accountLogin,
        byte[] characterName10,
        byte serverClass,
        CancellationToken ct)
    {
        var rows = await LoadRowsAsync(skillRepo, accountLogin, characterName10, ct).ConfigureAwait(false);
        if (rows.Count == 0)
        {
            rows = ToRows(CharacterSkillCatalog.GetDefaultEntries(serverClass));
            if (rows.Count > 0 && skillRepo is not null && !string.IsNullOrEmpty(accountLogin))
            {
                await skillRepo
                    .ReplaceAllAsync(accountLogin, CharacterName(characterName10), ToPersistenceRows(rows), ct)
                    .ConfigureAwait(false);
                Console.WriteLine(
                    "[m7] skill list seeded char={0} class=0x{1:X2} count={2}",
                    CharacterName(characterName10),
                    serverClass,
                    rows.Count);
            }
        }

        return rows.Count == 0
            ? MagicListWire602.BuildEmpty()
            : MagicListWire602.BuildFromRows(rows);
    }

    static async Task<List<SkillRowAdapter>> LoadRowsAsync(
        PostgresCharacterSkillRepository? skillRepo,
        string? accountLogin,
        byte[] characterName10,
        CancellationToken ct)
    {
        if (skillRepo is null || string.IsNullOrEmpty(accountLogin))
        {
            return [];
        }

        var loaded = await skillRepo
            .LoadByCharacterAsync(accountLogin, CharacterName(characterName10), ct)
            .ConfigureAwait(false);
        return ToRows(loaded);
    }

    static List<SkillRowAdapter> ToRows(IReadOnlyList<CharacterSkillRow> rows)
    {
        var list = new List<SkillRowAdapter>(rows.Count);
        foreach (var row in rows)
        {
            list.Add(new SkillRowAdapter(row.Slot, row.Type, row.Level));
        }

        return list;
    }

    static List<SkillRowAdapter> ToRows(IReadOnlyList<MagicListWire602.Entry> entries)
    {
        var list = new List<SkillRowAdapter>(entries.Count);
        foreach (var entry in entries)
        {
            list.Add(new SkillRowAdapter(entry.Index, entry.Type, entry.Level));
        }

        return list;
    }

    static string CharacterName(byte[] name10) =>
        Encoding.ASCII.GetString(name10.AsSpan(0, 10)).TrimEnd('\0');

    static CharacterSkillRow[] ToPersistenceRows(IReadOnlyList<SkillRowAdapter> rows)
    {
        var persisted = new CharacterSkillRow[rows.Count];
        for (var i = 0; i < rows.Count; i++)
        {
            var row = rows[i];
            persisted[i] = new CharacterSkillRow
            {
                Slot = row.Slot,
                Type = row.Type,
                Level = row.Level,
            };
        }

        return persisted;
    }

    sealed class SkillRowAdapter(byte slot, ushort type, byte level) : MagicListWire602.CharacterSkillRowLike
    {
        public byte Slot { get; } = slot;

        public ushort Type { get; } = type;

        public byte Level { get; } = level;
    }
}
