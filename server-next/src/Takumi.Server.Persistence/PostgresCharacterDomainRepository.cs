using Npgsql;
using NpgsqlTypes;

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
            SELECT character_name, server_class, level, map_id, pos_x, pos_y, angle,
                   current_hp, max_hp, current_mp, max_mp, zen,
                   current_shield, max_shield
            FROM character_domain
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
            del.Parameters.Add(new NpgsqlParameter("a", NpgsqlDbType.Text) { Value = accountLogin });
            await del.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        foreach (var row in rows)
        {
            await using var ins = new NpgsqlCommand(
                """
                INSERT INTO character_domain (
                    account_login, character_name, server_class, level, map_id, pos_x, pos_y, angle,
                    current_hp, max_hp, current_mp, max_mp, zen, current_shield, max_shield)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15)
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
            await ins.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    public async ValueTask DisposeAsync() => await this._dataSource.DisposeAsync().ConfigureAwait(false);
}
