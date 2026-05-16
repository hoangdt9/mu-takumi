using Npgsql;

namespace Takumi.Server.Persistence;

/// <summary>Reads/writes <c>public.warehouse_slot</c> (account-wide, 12-byte item blobs).</summary>
public sealed class PostgresWarehouseSlotRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresWarehouseSlotRepository(string connectionString) =>
        _dataSource = NpgsqlDataSource.Create(connectionString);

    public async Task<IReadOnlyList<InventorySlotRow>> LoadByAccountAsync(string accountLogin, CancellationToken ct = default)
    {
        var list = new List<InventorySlotRow>();
        await using var conn = await _dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            SELECT slot_idx, item
            FROM warehouse_slot
            WHERE account_login = $1
            ORDER BY slot_idx
            LIMIT 256
            """,
            conn);
        cmd.Parameters.AddWithValue(accountLogin);
        await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
        while (await reader.ReadAsync(ct).ConfigureAwait(false))
        {
            var si = reader.GetInt16(0);
            if (si is < 0 or > 255)
            {
                continue;
            }

            var blob = reader.GetFieldValue<byte[]>(1);
            if (blob.Length != 12)
            {
                continue;
            }

            list.Add(new InventorySlotRow { Slot = (byte)si, Item12 = blob });
        }

        return list;
    }

    public async Task UpsertSlotAsync(string accountLogin, byte slotIdx, byte[] item12, CancellationToken ct = default)
    {
        if (item12.Length != 12)
        {
            throw new ArgumentException("Item blob must be 12 bytes.", nameof(item12));
        }

        await using var conn = await _dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            INSERT INTO warehouse_slot (account_login, slot_idx, item, updated_at)
            VALUES ($1, $2, $3, now())
            ON CONFLICT (account_login, slot_idx)
            DO UPDATE SET item = EXCLUDED.item, updated_at = now()
            """,
            conn);
        cmd.Parameters.AddWithValue(accountLogin);
        cmd.Parameters.AddWithValue((short)slotIdx);
        cmd.Parameters.AddWithValue(item12);
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
    }

    public async Task DeleteSlotAsync(string accountLogin, byte slotIdx, CancellationToken ct = default)
    {
        await using var conn = await _dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            "DELETE FROM warehouse_slot WHERE account_login = $1 AND slot_idx = $2",
            conn);
        cmd.Parameters.AddWithValue(accountLogin);
        cmd.Parameters.AddWithValue((short)slotIdx);
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
    }

    public async Task ReplaceAccountSlotsAsync(string accountLogin, IReadOnlyList<InventorySlotRow> rows, CancellationToken ct = default)
    {
        await using var conn = await _dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var tx = await conn.BeginTransactionAsync(ct).ConfigureAwait(false);
        await using (var del = new NpgsqlCommand("DELETE FROM warehouse_slot WHERE account_login = $1", conn, tx))
        {
            del.Parameters.AddWithValue(accountLogin);
            await del.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        foreach (var row in rows)
        {
            if (row.Item12.Length != 12)
            {
                continue;
            }

            await using var ins = new NpgsqlCommand(
                """
                INSERT INTO warehouse_slot (account_login, slot_idx, item, updated_at)
                VALUES ($1, $2, $3, now())
                """,
                conn,
                tx);
            ins.Parameters.AddWithValue(accountLogin);
            ins.Parameters.AddWithValue((short)row.Slot);
            ins.Parameters.AddWithValue(row.Item12);
            await ins.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    public async ValueTask DisposeAsync() => await _dataSource.DisposeAsync().ConfigureAwait(false);
}
