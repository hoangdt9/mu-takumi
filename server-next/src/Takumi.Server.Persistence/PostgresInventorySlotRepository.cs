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
        cmd.Parameters.Add(new NpgsqlParameter("a", NpgsqlDbType.Text) { Value = accountLogin });
        cmd.Parameters.Add(new NpgsqlParameter("n", NpgsqlDbType.Text) { Value = name });
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

    public async ValueTask DisposeAsync() => await this._dataSource.DisposeAsync().ConfigureAwait(false);
}
