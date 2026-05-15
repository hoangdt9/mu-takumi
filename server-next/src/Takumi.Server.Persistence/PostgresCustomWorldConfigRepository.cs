using System.Text.Json;
using Npgsql;
using NpgsqlTypes;

namespace Takumi.Server.Persistence;

public sealed class PostgresCustomWorldConfigRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresCustomWorldConfigRepository(string connectionString) =>
        this._dataSource = NpgsqlDataSource.Create(connectionString);

    public async Task<IReadOnlyList<CustomWorldConfigRow>> LoadAllAsync(CancellationToken ct = default)
    {
        var list = new List<CustomWorldConfigRow>(32);
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            "SELECT config_key, format, payload::text FROM custom_world_config ORDER BY config_key",
            conn);
        await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
        while (await reader.ReadAsync(ct).ConfigureAwait(false))
        {
            list.Add(
                new CustomWorldConfigRow
                {
                    ConfigKey = reader.GetString(0),
                    Format = reader.GetString(1),
                    PayloadJson = reader.GetString(2),
                });
        }

        return list;
    }

    public async Task ReplaceAllAsync(IReadOnlyList<CustomWorldConfigRow> rows, CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var tx = await conn.BeginTransactionAsync(ct).ConfigureAwait(false);
        await using (var del = new NpgsqlCommand("DELETE FROM custom_world_config", conn, tx))
        {
            await del.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        foreach (var r in rows)
        {
            await using var cmd = new NpgsqlCommand(
                """
                INSERT INTO custom_world_config (config_key, format, payload, source_file)
                VALUES (@key, @format, @payload::jsonb, @source)
                """,
                conn,
                tx);
            cmd.Parameters.AddWithValue("key", r.ConfigKey);
            cmd.Parameters.AddWithValue("format", r.Format);
            cmd.Parameters.Add("payload", NpgsqlDbType.Jsonb).Value = r.PayloadJson;
            cmd.Parameters.AddWithValue("source", r.ConfigKey);
            await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    public ValueTask DisposeAsync() => this._dataSource.DisposeAsync();
}
