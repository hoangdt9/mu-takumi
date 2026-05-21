using Npgsql;

namespace Takumi.Server.Persistence;

/// <summary>Reads/writes <c>public.character_skill</c> (see <c>sql/patches/016_character_skill.sql</c>).</summary>
public sealed class PostgresCharacterSkillRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresCharacterSkillRepository(string connectionString) =>
        this._dataSource = NpgsqlDataSource.Create(connectionString);

    public async Task<IReadOnlyList<CharacterSkillRow>> LoadByCharacterAsync(
        string accountLogin,
        string characterName,
        CancellationToken ct = default)
    {
        try
        {
            return await LoadByCharacterCoreAsync(accountLogin, characterName, ct).ConfigureAwait(false);
        }
        catch (PostgresException ex) when (ex.SqlState == PostgresErrorCodes.UndefinedTable)
        {
            return [];
        }
    }

    async Task<IReadOnlyList<CharacterSkillRow>> LoadByCharacterCoreAsync(
        string accountLogin,
        string characterName,
        CancellationToken ct)
    {
        var name = CharacterRosterMerge.NormaliseName(characterName);
        var list = new List<CharacterSkillRow>();
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            SELECT skill_slot, skill_type, skill_level
            FROM character_skill
            WHERE account_login = $1 AND character_name = $2
            ORDER BY skill_slot
            LIMIT 255
            """,
            conn);
        cmd.Parameters.AddWithValue(accountLogin);
        cmd.Parameters.AddWithValue(name);
        await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
        while (await reader.ReadAsync(ct).ConfigureAwait(false))
        {
            // Wire F3 11 uses a byte index; MG combat QA uses compact slots 1..30 (see CharacterSkillCatalog).
            var slot = reader.GetInt16(0);
            if (slot is < 0 or > 255)
            {
                continue;
            }

            var type = reader.GetInt16(1);
            if (type <= 0)
            {
                continue;
            }

            var level = reader.GetInt16(2);
            list.Add(new CharacterSkillRow
            {
                Slot = (byte)slot,
                Type = (ushort)Math.Clamp((int)type, 1, ushort.MaxValue),
                Level = (byte)Math.Clamp((int)level, 0, byte.MaxValue),
            });
        }

        return list;
    }

    public async Task ReplaceAllAsync(
        string accountLogin,
        string characterName,
        IReadOnlyList<CharacterSkillRow> skills,
        CancellationToken ct = default)
    {
        try
        {
            await ReplaceAllCoreAsync(accountLogin, characterName, skills, ct).ConfigureAwait(false);
        }
        catch (PostgresException ex) when (ex.SqlState == PostgresErrorCodes.UndefinedTable)
        {
            Console.Error.WriteLine(
                "[postgres-mirror] character_skill table missing (sql/patches/016_character_skill.sql) — learned skills won't survive relog.");
        }
    }

    async Task ReplaceAllCoreAsync(
        string accountLogin,
        string characterName,
        IReadOnlyList<CharacterSkillRow> skills,
        CancellationToken ct)
    {
        var name = CharacterRosterMerge.NormaliseName(characterName);
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var tx = await conn.BeginTransactionAsync(ct).ConfigureAwait(false);

        await using (var del = new NpgsqlCommand(
                         """
                         DELETE FROM character_skill
                         WHERE account_login = $1 AND character_name = $2
                         """,
                         conn,
                         tx))
        {
            del.Parameters.AddWithValue(accountLogin);
            del.Parameters.AddWithValue(name);
            await del.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        foreach (var skill in skills)
        {
            await using var ins = new NpgsqlCommand(
                """
                INSERT INTO character_skill (account_login, character_name, skill_slot, skill_type, skill_level, updated_at)
                VALUES ($1, $2, $3, $4, $5, now())
                """,
                conn,
                tx);
            ins.Parameters.AddWithValue(accountLogin);
            ins.Parameters.AddWithValue(name);
            ins.Parameters.AddWithValue((short)skill.Slot);
            ins.Parameters.AddWithValue((short)skill.Type);
            ins.Parameters.AddWithValue((short)skill.Level);
            await ins.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    public ValueTask DisposeAsync() => this._dataSource.DisposeAsync();
}
