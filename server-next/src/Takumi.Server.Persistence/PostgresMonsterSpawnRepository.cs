using Npgsql;
using NpgsqlTypes;

namespace Takumi.Server.Persistence;

/// <summary>Reads/writes <c>public.monster_spawn</c> for M8 world ETL.</summary>
public sealed class PostgresMonsterSpawnRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresMonsterSpawnRepository(string connectionString) =>
        this._dataSource = NpgsqlDataSource.Create(connectionString);

    public async Task<IReadOnlyList<MonsterSpawnRow>> LoadAllAsync(CancellationToken ct = default)
    {
        var list = new List<MonsterSpawnRow>(1024);
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            SELECT id, spawn_type, monster_class, map_id, dis, pos_x, pos_y, range_tx, range_ty, dir, spawn_value
            FROM monster_spawn
            ORDER BY map_id, id
            """,
            conn);
        await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
        while (await reader.ReadAsync(ct).ConfigureAwait(false))
        {
            list.Add(
                new MonsterSpawnRow
                {
                    Id = reader.GetInt32(0),
                    SpawnType = reader.GetInt16(1),
                    MonsterClass = reader.GetInt32(2),
                    MapId = (byte)reader.GetInt16(3),
                    Dis = reader.GetInt32(4),
                    PosX = reader.GetInt16(5),
                    PosY = reader.GetInt16(6),
                    RangeTx = reader.GetInt16(7),
                    RangeTy = reader.GetInt16(8),
                    Dir = reader.GetInt16(9),
                    SpawnValue = reader.GetInt32(10),
                });
        }

        return list;
    }

    public async Task ReplaceAllAsync(IReadOnlyList<MonsterSpawnRow> rows, string? sourceFile, CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var tx = await conn.BeginTransactionAsync(ct).ConfigureAwait(false);

        await using (var del = new NpgsqlCommand("DELETE FROM monster_spawn", conn, tx))
        {
            await del.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        if (rows.Count == 0)
        {
            await tx.CommitAsync(ct).ConfigureAwait(false);
            return;
        }

        await using (var copy = await conn.BeginBinaryImportAsync(
                         "COPY monster_spawn (spawn_type, monster_class, map_id, dis, pos_x, pos_y, range_tx, range_ty, dir, spawn_value, source_file) FROM STDIN (FORMAT BINARY)",
                         ct)
                     .ConfigureAwait(false))
        {
            foreach (var r in rows)
            {
                await copy.StartRowAsync(ct).ConfigureAwait(false);
                await copy.WriteAsync(r.SpawnType, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(r.MonsterClass, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
                await copy.WriteAsync((short)r.MapId, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(r.Dis, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
                await copy.WriteAsync(r.PosX, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(r.PosY, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(r.RangeTx, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(r.RangeTy, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(r.Dir, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
                await copy.WriteAsync(r.SpawnValue, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
                if (sourceFile is null)
                {
                    await copy.WriteNullAsync(ct).ConfigureAwait(false);
                }
                else
                {
                    await copy.WriteAsync(sourceFile, NpgsqlDbType.Text, ct).ConfigureAwait(false);
                }
            }

            await copy.CompleteAsync(ct).ConfigureAwait(false);
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    public ValueTask DisposeAsync() => this._dataSource.DisposeAsync();
}
