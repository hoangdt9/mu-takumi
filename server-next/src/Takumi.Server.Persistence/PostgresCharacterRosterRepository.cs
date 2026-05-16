using Npgsql;
using NpgsqlTypes;

namespace Takumi.Server.Persistence;

/// <summary>Stores per-account character roster in Postgres (<c>public.character_roster</c>) for M4b JSON↔DB sync.</summary>
public sealed class PostgresCharacterRosterRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresCharacterRosterRepository(string connectionString) =>
        this._dataSource = NpgsqlDataSource.Create(connectionString);

    public static string? BuildConnectionStringFromEnv()
    {
        var direct = Environment.GetEnvironmentVariable("TAKUMI_PG_CONNECTION_STRING")?.Trim();
        if (!string.IsNullOrEmpty(direct))
        {
            return direct;
        }

        var host = Environment.GetEnvironmentVariable("TAKUMI_PG_HOST")?.Trim();
        if (string.IsNullOrEmpty(host))
        {
            return null;
        }

        var port = Environment.GetEnvironmentVariable("TAKUMI_PG_PORT")?.Trim();
        if (!int.TryParse(port, System.Globalization.NumberStyles.Integer, System.Globalization.CultureInfo.InvariantCulture, out var p) || p <= 0)
        {
            p = 5432;
        }

        var user = Environment.GetEnvironmentVariable("TAKUMI_PG_USER")?.Trim() ?? "takumi";
        var password = Environment.GetEnvironmentVariable("TAKUMI_PG_PASSWORD")?.Trim() ?? "takumi";
        var database = Environment.GetEnvironmentVariable("TAKUMI_PG_DATABASE")?.Trim() ?? "takumi_runtime";
        return $"Host={host};Port={p};Username={user};Password={password};Database={database}";
    }

    public async Task<IReadOnlyList<CharacterRosterRow>> LoadByAccountAsync(string accountLogin, CancellationToken ct = default)
    {
        var list = new List<CharacterRosterRow>();
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            SELECT character_name, server_class, level, map_id, pos_x, pos_y, angle,
                   current_hp, max_hp, current_mp, max_mp, zen,
                   current_shield, max_shield,
                   strength, dexterity, vitality, energy, leadership, level_up_point,
                   current_bp, max_bp
            FROM character_roster
            WHERE account_login = $1
            ORDER BY character_name
            """,
            conn);
        cmd.Parameters.Add(new NpgsqlParameter("a", NpgsqlDbType.Text) { Value = accountLogin });
        await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
        while (await reader.ReadAsync(ct).ConfigureAwait(false))
        {
            list.Add(
                new CharacterRosterRow
                {
                    Name = reader.GetString(0),
                    ServerClass = (byte)reader.GetInt16(1),
                    Level = (ushort)reader.GetInt32(2),
                    MapId = (byte)reader.GetInt16(3),
                    PosX = (byte)reader.GetInt16(4),
                    PosY = (byte)reader.GetInt16(5),
                    Angle = (byte)reader.GetInt16(6),
                    CurrentHp = reader.GetInt32(7),
                    MaxHp = reader.GetInt32(8),
                    CurrentMp = reader.GetInt32(9),
                    MaxMp = reader.GetInt32(10),
                    Zen = reader.GetInt64(11),
                    CurrentShield = reader.GetInt32(12),
                    MaxShield = reader.GetInt32(13),
                    Strength = reader.GetInt32(14),
                    Dexterity = reader.GetInt32(15),
                    Vitality = reader.GetInt32(16),
                    Energy = reader.GetInt32(17),
                    Leadership = reader.GetInt32(18),
                    LevelUpPoint = reader.GetInt32(19),
                    CurrentBp = reader.GetInt32(20),
                    MaxBp = reader.GetInt32(21),
                });
        }

        return list;
    }

    /// <summary>Replace all rows for the account with the given snapshot (matches JSON save semantics).</summary>
    public async Task ReplaceAccountRosterAsync(string accountLogin, IReadOnlyList<CharacterRosterRow> rows, CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var tx = await conn.BeginTransactionAsync(ct).ConfigureAwait(false);
        await using (var del = new NpgsqlCommand("DELETE FROM character_roster WHERE account_login = $1", conn, tx))
        {
            del.Parameters.Add(new NpgsqlParameter("a", NpgsqlDbType.Text) { Value = accountLogin });
            await del.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        foreach (var row in rows)
        {
            await using var ins = new NpgsqlCommand(
                """
                INSERT INTO character_roster (
                    account_login, character_name, server_class, level, map_id, pos_x, pos_y, angle,
                    current_hp, max_hp, current_mp, max_mp, zen, current_shield, max_shield,
                    strength, dexterity, vitality, energy, leadership, level_up_point, current_bp, max_bp)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16, $17, $18, $19, $20, $21, $22, $23)
                """,
                conn,
                tx);
            ins.Parameters.Add(new NpgsqlParameter("a", NpgsqlDbType.Text) { Value = accountLogin });
            ins.Parameters.Add(new NpgsqlParameter("n", NpgsqlDbType.Text) { Value = CharacterRosterMerge.NormaliseName(row.Name) });
            ins.Parameters.Add(new NpgsqlParameter("c", NpgsqlDbType.Smallint) { Value = (short)row.ServerClass });
            ins.Parameters.Add(new NpgsqlParameter("l", NpgsqlDbType.Integer) { Value = (int)row.Level });
            ins.Parameters.Add(new NpgsqlParameter("m", NpgsqlDbType.Smallint) { Value = (short)row.MapId });
            ins.Parameters.Add(new NpgsqlParameter("x", NpgsqlDbType.Smallint) { Value = (short)row.PosX });
            ins.Parameters.Add(new NpgsqlParameter("y", NpgsqlDbType.Smallint) { Value = (short)row.PosY });
            ins.Parameters.Add(new NpgsqlParameter("g", NpgsqlDbType.Smallint) { Value = (short)row.Angle });
            ins.Parameters.Add(new NpgsqlParameter("hp", NpgsqlDbType.Integer) { Value = row.CurrentHp });
            ins.Parameters.Add(new NpgsqlParameter("hpmax", NpgsqlDbType.Integer) { Value = row.MaxHp });
            ins.Parameters.Add(new NpgsqlParameter("mp", NpgsqlDbType.Integer) { Value = row.CurrentMp });
            ins.Parameters.Add(new NpgsqlParameter("mpmax", NpgsqlDbType.Integer) { Value = row.MaxMp });
            ins.Parameters.Add(new NpgsqlParameter("z", NpgsqlDbType.Bigint) { Value = row.Zen });
            ins.Parameters.Add(new NpgsqlParameter("sd", NpgsqlDbType.Integer) { Value = row.CurrentShield });
            ins.Parameters.Add(new NpgsqlParameter("sdm", NpgsqlDbType.Integer) { Value = row.MaxShield });
            ins.Parameters.Add(new NpgsqlParameter("str", NpgsqlDbType.Integer) { Value = row.Strength });
            ins.Parameters.Add(new NpgsqlParameter("dex", NpgsqlDbType.Integer) { Value = row.Dexterity });
            ins.Parameters.Add(new NpgsqlParameter("vit", NpgsqlDbType.Integer) { Value = row.Vitality });
            ins.Parameters.Add(new NpgsqlParameter("ene", NpgsqlDbType.Integer) { Value = row.Energy });
            ins.Parameters.Add(new NpgsqlParameter("lead", NpgsqlDbType.Integer) { Value = row.Leadership });
            ins.Parameters.Add(new NpgsqlParameter("lup", NpgsqlDbType.Integer) { Value = row.LevelUpPoint });
            ins.Parameters.Add(new NpgsqlParameter("bp", NpgsqlDbType.Integer) { Value = row.CurrentBp });
            ins.Parameters.Add(new NpgsqlParameter("bpmax", NpgsqlDbType.Integer) { Value = row.MaxBp });
            await ins.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    public async Task UpdateZenAsync(
        string accountLogin,
        string characterName,
        long zen,
        CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            UPDATE character_roster
            SET zen = $3, updated_at = now()
            WHERE account_login = $1 AND character_name = $2
            """,
            conn);
        cmd.Parameters.Add(new NpgsqlParameter("a", NpgsqlDbType.Text) { Value = accountLogin });
        cmd.Parameters.Add(new NpgsqlParameter("n", NpgsqlDbType.Text) { Value = CharacterRosterMerge.NormaliseName(characterName) });
        cmd.Parameters.Add(new NpgsqlParameter("z", NpgsqlDbType.Bigint) { Value = zen });
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
    }

    public async Task UpsertVitalsAsync(
        string accountLogin,
        string characterName,
        int currentHp,
        int maxHp,
        int currentMp,
        int maxMp,
        int currentShield = 0,
        int maxShield = 0,
        CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            UPDATE character_roster
            SET current_hp = $3, max_hp = $4, current_mp = $5, max_mp = $6,
                current_shield = $7, max_shield = $8, updated_at = NOW()
            WHERE account_login = $1 AND character_name = $2
            """,
            conn);
        cmd.Parameters.Add(new NpgsqlParameter("a", NpgsqlDbType.Text) { Value = accountLogin });
        cmd.Parameters.Add(new NpgsqlParameter("n", NpgsqlDbType.Text) { Value = CharacterRosterMerge.NormaliseName(characterName) });
        cmd.Parameters.Add(new NpgsqlParameter("hp", NpgsqlDbType.Integer) { Value = currentHp });
        cmd.Parameters.Add(new NpgsqlParameter("hpmax", NpgsqlDbType.Integer) { Value = maxHp });
        cmd.Parameters.Add(new NpgsqlParameter("mp", NpgsqlDbType.Integer) { Value = currentMp });
        cmd.Parameters.Add(new NpgsqlParameter("mpmax", NpgsqlDbType.Integer) { Value = maxMp });
        cmd.Parameters.Add(new NpgsqlParameter("sd", NpgsqlDbType.Integer) { Value = currentShield });
        cmd.Parameters.Add(new NpgsqlParameter("sdm", NpgsqlDbType.Integer) { Value = maxShield });
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
    }

    public async Task DeleteCharacterAsync(string accountLogin, string characterName, CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            "DELETE FROM character_roster WHERE account_login = $1 AND character_name = $2",
            conn);
        cmd.Parameters.Add(new NpgsqlParameter("a", NpgsqlDbType.Text) { Value = accountLogin });
        cmd.Parameters.Add(new NpgsqlParameter("n", NpgsqlDbType.Text) { Value = CharacterRosterMerge.NormaliseName(characterName) });
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
    }

    public async ValueTask DisposeAsync() => await this._dataSource.DisposeAsync().ConfigureAwait(false);
}
