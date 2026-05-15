using Npgsql;
using NpgsqlTypes;

namespace Takumi.Server.Persistence;

public sealed class PostgresMapGateRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresMapGateRepository(string connectionString) =>
        this._dataSource = NpgsqlDataSource.Create(connectionString);

    public async Task<IReadOnlyList<MapGateRow>> LoadAllAsync(CancellationToken ct = default)
    {
        var list = new List<MapGateRow>(512);
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            SELECT id, gate_index, flag, map_id, pos_x, pos_y, range_tx, range_ty, target_gate, dir,
                   min_level, max_level, min_reset, max_reset, account_level
            FROM map_gate
            ORDER BY gate_index
            """,
            conn);
        await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
        while (await reader.ReadAsync(ct).ConfigureAwait(false))
        {
            list.Add(ReadRow(reader));
        }

        return list;
    }

    public async Task ReplaceAllAsync(IReadOnlyList<MapGateRow> rows, string? sourceFile, CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var tx = await conn.BeginTransactionAsync(ct).ConfigureAwait(false);
        await using (var del = new NpgsqlCommand("DELETE FROM map_gate", conn, tx))
        {
            await del.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        if (rows.Count == 0)
        {
            await tx.CommitAsync(ct).ConfigureAwait(false);
            return;
        }

        await using var copy = await conn.BeginBinaryImportAsync(
            """
            COPY map_gate (gate_index, flag, map_id, pos_x, pos_y, range_tx, range_ty, target_gate, dir,
                           min_level, max_level, min_reset, max_reset, account_level, source_file)
            FROM STDIN (FORMAT BINARY)
            """,
            ct).ConfigureAwait(false);

        foreach (var r in rows)
        {
            await copy.StartRowAsync(ct).ConfigureAwait(false);
            await copy.WriteAsync(r.GateIndex, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.Flag, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
            await copy.WriteAsync((short)r.MapId, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.PosX, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.PosY, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.RangeTx, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.RangeTy, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.TargetGate, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.Dir, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.MinLevel, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.MaxLevel, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.MinReset, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.MaxReset, NpgsqlDbType.Integer, ct).ConfigureAwait(false);
            await copy.WriteAsync(r.AccountLevel, NpgsqlDbType.Smallint, ct).ConfigureAwait(false);
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
        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    static MapGateRow ReadRow(NpgsqlDataReader reader) =>
        new()
        {
            Id = reader.GetInt32(0),
            GateIndex = reader.GetInt32(1),
            Flag = reader.GetInt16(2),
            MapId = (byte)reader.GetInt16(3),
            PosX = reader.GetInt16(4),
            PosY = reader.GetInt16(5),
            RangeTx = reader.GetInt16(6),
            RangeTy = reader.GetInt16(7),
            TargetGate = reader.GetInt32(8),
            Dir = reader.GetInt16(9),
            MinLevel = reader.GetInt32(10),
            MaxLevel = reader.GetInt32(11),
            MinReset = reader.GetInt32(12),
            MaxReset = reader.GetInt32(13),
            AccountLevel = reader.GetInt16(14),
        };

    public ValueTask DisposeAsync() => this._dataSource.DisposeAsync();
}
