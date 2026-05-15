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
                   current_hp, max_hp, current_mp, max_mp, zen
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
                    MapId = (byte)reader.GetInt16(3),
                    PosX = (byte)reader.GetInt16(4),
                    PosY = (byte)reader.GetInt16(5),
                    Angle = (byte)reader.GetInt16(6),
                    CurrentHp = reader.GetInt32(7),
                    MaxHp = reader.GetInt32(8),
                    CurrentMp = reader.GetInt32(9),
                    MaxMp = reader.GetInt32(10),
                    Zen = reader.GetInt64(11),
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
                    account_login, character_name, server_class, level, map_id, pos_x, pos_y, angle,
                    current_hp, max_hp, current_mp, max_mp, zen)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
                """,
                conn,
                tx);
            ins.Parameters.AddWithValue(accountLogin);
            ins.Parameters.AddWithValue(CharacterRosterMerge.NormaliseName(row.Name));
            ins.Parameters.AddWithValue((short)row.ServerClass);
            ins.Parameters.AddWithValue((int)row.Level);
            ins.Parameters.AddWithValue((short)row.MapId);
            ins.Parameters.AddWithValue((short)row.PosX);
            ins.Parameters.AddWithValue((short)row.PosY);
            ins.Parameters.AddWithValue((short)row.Angle);
            ins.Parameters.AddWithValue(row.CurrentHp);
            ins.Parameters.AddWithValue(row.MaxHp);
            ins.Parameters.AddWithValue(row.CurrentMp);
            ins.Parameters.AddWithValue(row.MaxMp);
            ins.Parameters.AddWithValue(row.Zen);
            await ins.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    public async ValueTask DisposeAsync() => await this._dataSource.DisposeAsync().ConfigureAwait(false);
}
