using Npgsql;
using NpgsqlTypes;

namespace Takumi.Server.Persistence;

/// <summary>Reads <c>public.inventory_slot</c> (see <c>sql/init/002_inventory_slot.sql</c>).</summary>
public sealed class PostgresInventorySlotRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresInventorySlotRepository(string connectionString) =>
        this._dataSource = NpgsqlDataSource.Create(connectionString);

    /// <summary>Load all non-empty slots for one character (ordered by slot index).</summary>
    public async Task<IReadOnlyList<InventorySlotRow>> LoadByCharacterAsync(
        string accountLogin,
        string characterName,
        CancellationToken ct = default)
    {
        var name = CharacterRosterMerge.NormaliseName(characterName);
        var list = new List<InventorySlotRow>();
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            SELECT slot_idx, item
            FROM inventory_slot
            WHERE account_login = $1 AND character_name = $2
            ORDER BY slot_idx
            LIMIT 255
            """,
            conn);
        cmd.Parameters.AddWithValue(accountLogin);
        cmd.Parameters.AddWithValue(name);
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

    public async Task UpsertSlotAsync(
        string accountLogin,
        string characterName,
        byte slotIdx,
        byte[] item12,
        CancellationToken ct = default)
    {
        if (item12.Length != 12)
        {
            throw new ArgumentException("Item blob must be 12 bytes.", nameof(item12));
        }

        var name = CharacterRosterMerge.NormaliseName(characterName);
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            INSERT INTO inventory_slot (account_login, character_name, slot_idx, item, updated_at)
            VALUES ($1, $2, $3, $4, now())
            ON CONFLICT (account_login, character_name, slot_idx)
            DO UPDATE SET item = EXCLUDED.item, updated_at = now()
            """,
            conn);
        cmd.Parameters.AddWithValue(accountLogin);
        cmd.Parameters.AddWithValue(name);
        cmd.Parameters.AddWithValue((short)slotIdx);
        cmd.Parameters.AddWithValue(item12);
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
    }

    public async Task DeleteSlotAsync(
        string accountLogin,
        string characterName,
        byte slotIdx,
        CancellationToken ct = default)
    {
        var name = CharacterRosterMerge.NormaliseName(characterName);
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            DELETE FROM inventory_slot
            WHERE account_login = $1 AND character_name = $2 AND slot_idx = $3
            """,
            conn);
        cmd.Parameters.AddWithValue(accountLogin);
        cmd.Parameters.AddWithValue(name);
        cmd.Parameters.AddWithValue((short)slotIdx);
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
    }

    public async Task ReplaceCharacterSlotsAsync(
        string accountLogin,
        string characterName,
        IReadOnlyDictionary<byte, byte[]> slots,
        CancellationToken ct = default)
    {
        var rows = slots
            .Select(kv => new InventorySlotRow { Slot = kv.Key, Item12 = kv.Value })
            .ToList();
        await ReplaceCharacterSlotsAsync(accountLogin, characterName, rows, ct).ConfigureAwait(false);
    }

    /// <summary>Replace all slots for one character (matches in-memory bag after shop session).</summary>
    public async Task ReplaceCharacterSlotsAsync(
        string accountLogin,
        string characterName,
        IReadOnlyList<InventorySlotRow> rows,
        CancellationToken ct = default)
    {
        var name = CharacterRosterMerge.NormaliseName(characterName);
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var tx = await conn.BeginTransactionAsync(ct).ConfigureAwait(false);
        await using (var del = new NpgsqlCommand(
            "DELETE FROM inventory_slot WHERE account_login = $1 AND character_name = $2",
            conn,
            tx))
        {
            del.Parameters.AddWithValue(accountLogin);
            del.Parameters.AddWithValue(name);
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
                INSERT INTO inventory_slot (account_login, character_name, slot_idx, item, updated_at)
                VALUES ($1, $2, $3, $4, now())
                """,
                conn,
                tx);
            ins.Parameters.AddWithValue(accountLogin);
            ins.Parameters.AddWithValue(name);
            ins.Parameters.AddWithValue((short)row.Slot);
            ins.Parameters.AddWithValue(row.Item12);
            await ins.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    public async ValueTask DisposeAsync() => await this._dataSource.DisposeAsync().ConfigureAwait(false);
}
