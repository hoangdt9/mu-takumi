using Npgsql;

namespace Takumi.Server.Persistence;

/// <summary>M4b domain mirror: <c>public.character_domain</c> (synced from <c>character_roster</c> writes).</summary>
public sealed class PostgresCharacterDomainRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresCharacterDomainRepository(string connectionString) =>
        this._dataSource = NpgsqlDataSource.Create(connectionString);

    public async Task<IReadOnlyList<CharacterRosterRow>> LoadByAccountAsync(string accountLogin, CancellationToken ct = default)
    {
        var list = new List<CharacterRosterRow>();
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            SELECT character_name, server_class, level, experience, map_id, pos_x, pos_y, angle,
                   current_hp, max_hp, current_mp, max_mp, zen,
                   current_shield, max_shield,
                   strength, dexterity, vitality, energy, leadership, level_up_point,
                   current_bp, max_bp
            FROM character_domain
            WHERE account_login = $1
            ORDER BY character_name
            """,
            conn);
        cmd.Parameters.AddWithValue(accountLogin);
        await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
        while (await reader.ReadAsync(ct).ConfigureAwait(false))
        {
            list.Add(
                new CharacterRosterRow
                {
                    Name = reader.GetString(0),
                    ServerClass = (byte)reader.GetInt16(1),
                    Level = (ushort)reader.GetInt32(2),
                    Experience = reader.GetInt64(3),
                    MapId = (byte)reader.GetInt16(4),
                    PosX = (byte)reader.GetInt16(5),
                    PosY = (byte)reader.GetInt16(6),
                    Angle = (byte)reader.GetInt16(7),
                    CurrentHp = reader.GetInt32(8),
                    MaxHp = reader.GetInt32(9),
                    CurrentMp = reader.GetInt32(10),
                    MaxMp = reader.GetInt32(11),
                    Zen = reader.GetInt64(12),
                    CurrentShield = reader.GetInt32(13),
                    MaxShield = reader.GetInt32(14),
                    Strength = reader.GetInt32(15),
                    Dexterity = reader.GetInt32(16),
                    Vitality = reader.GetInt32(17),
                    Energy = reader.GetInt32(18),
                    Leadership = reader.GetInt32(19),
                    LevelUpPoint = reader.GetInt32(20),
                    CurrentBp = reader.GetInt32(21),
                    MaxBp = reader.GetInt32(22),
                });
        }

        return list;
    }

    public async Task ReplaceAccountAsync(string accountLogin, IReadOnlyList<CharacterRosterRow> rows, CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var tx = await conn.BeginTransactionAsync(ct).ConfigureAwait(false);
        await using (var del = new NpgsqlCommand("DELETE FROM character_domain WHERE account_login = $1", conn, tx))
        {
            del.Parameters.AddWithValue(accountLogin);
            await del.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        foreach (var row in rows)
        {
            await using var ins = new NpgsqlCommand(
                """
                INSERT INTO character_domain (
                    account_login, character_name, server_class, level, experience, map_id, pos_x, pos_y, angle,
                    current_hp, max_hp, current_mp, max_mp, zen, current_shield, max_shield,
                    strength, dexterity, vitality, energy, leadership, level_up_point, current_bp, max_bp)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16, $17, $18, $19, $20, $21, $22, $23, $24)
                """,
                conn,
                tx);
            ins.Parameters.AddWithValue(accountLogin);
            ins.Parameters.AddWithValue(CharacterRosterMerge.NormaliseName(row.Name));
            ins.Parameters.AddWithValue((short)row.ServerClass);
            ins.Parameters.AddWithValue((int)row.Level);
            ins.Parameters.AddWithValue(row.Experience);
            ins.Parameters.AddWithValue((short)row.MapId);
            ins.Parameters.AddWithValue((short)row.PosX);
            ins.Parameters.AddWithValue((short)row.PosY);
            ins.Parameters.AddWithValue((short)row.Angle);
            ins.Parameters.AddWithValue(row.CurrentHp);
            ins.Parameters.AddWithValue(row.MaxHp);
            ins.Parameters.AddWithValue(row.CurrentMp);
            ins.Parameters.AddWithValue(row.MaxMp);
            ins.Parameters.AddWithValue(row.Zen);
            ins.Parameters.AddWithValue(row.CurrentShield);
            ins.Parameters.AddWithValue(row.MaxShield);
            ins.Parameters.AddWithValue(row.Strength);
            ins.Parameters.AddWithValue(row.Dexterity);
            ins.Parameters.AddWithValue(row.Vitality);
            ins.Parameters.AddWithValue(row.Energy);
            ins.Parameters.AddWithValue(row.Leadership);
            ins.Parameters.AddWithValue(row.LevelUpPoint);
            ins.Parameters.AddWithValue(row.CurrentBp);
            ins.Parameters.AddWithValue(row.MaxBp);
            await ins.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    public async Task UpsertProgressAsync(
        string accountLogin,
        string characterName,
        ushort level,
        long experience,
        int levelUpPoint,
        int currentHp,
        int maxHp,
        int currentMp,
        int maxMp,
        int currentShield = 0,
        int maxShield = 0,
        int strength = 0,
        int dexterity = 0,
        int vitality = 0,
        int energy = 0,
        int leadership = 0,
        int currentBp = 0,
        int maxBp = 0,
        CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            UPDATE character_domain
            SET level = $3, experience = $4, level_up_point = $5,
                current_hp = $6, max_hp = $7, current_mp = $8, max_mp = $9,
                current_shield = $10, max_shield = $11,
                strength = $12, dexterity = $13, vitality = $14, energy = $15, leadership = $16,
                current_bp = $17, max_bp = $18,
                updated_at = NOW()
            WHERE account_login = $1 AND character_name = $2
            """,
            conn);
        cmd.Parameters.AddWithValue(accountLogin);
        cmd.Parameters.AddWithValue(CharacterRosterMerge.NormaliseName(characterName));
        cmd.Parameters.AddWithValue((int)level);
        cmd.Parameters.AddWithValue(experience);
        cmd.Parameters.AddWithValue(levelUpPoint);
        cmd.Parameters.AddWithValue(currentHp);
        cmd.Parameters.AddWithValue(maxHp);
        cmd.Parameters.AddWithValue(currentMp);
        cmd.Parameters.AddWithValue(maxMp);
        cmd.Parameters.AddWithValue(currentShield);
        cmd.Parameters.AddWithValue(maxShield);
        cmd.Parameters.AddWithValue(strength);
        cmd.Parameters.AddWithValue(dexterity);
        cmd.Parameters.AddWithValue(vitality);
        cmd.Parameters.AddWithValue(energy);
        cmd.Parameters.AddWithValue(leadership);
        cmd.Parameters.AddWithValue(currentBp);
        cmd.Parameters.AddWithValue(maxBp);
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
    }

    public async ValueTask DisposeAsync() => await this._dataSource.DisposeAsync().ConfigureAwait(false);
}
