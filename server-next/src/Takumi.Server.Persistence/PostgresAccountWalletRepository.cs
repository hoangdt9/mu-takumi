using Npgsql;

namespace Takumi.Server.Persistence;

/// <summary>Reads/writes <c>account.warehouse_zen</c> and coin columns.</summary>
public sealed class PostgresAccountWalletRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresAccountWalletRepository(string connectionString) =>
        _dataSource = NpgsqlDataSource.Create(connectionString);

    public async Task<AccountWalletRow?> TryLoadAsync(string accountLogin, CancellationToken ct = default)
    {
        var login = NormaliseLogin(accountLogin);
        await using var conn = await _dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            SELECT warehouse_zen, wcoin_c, wcoin_p, goblin_point
            FROM account
            WHERE account_login = $1
            LIMIT 1
            """,
            conn);
        cmd.Parameters.AddWithValue(login);
        await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
        if (!await reader.ReadAsync(ct).ConfigureAwait(false))
        {
            return null;
        }

        return new AccountWalletRow(
            reader.GetInt64(0),
            reader.GetInt64(1),
            reader.GetInt64(2),
            reader.GetInt64(3));
    }

    public async Task SaveAsync(string accountLogin, AccountWalletRow row, CancellationToken ct = default)
    {
        var login = NormaliseLogin(accountLogin);
        await using var conn = await _dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            UPDATE account
            SET warehouse_zen = $2,
                wcoin_c = $3,
                wcoin_p = $4,
                goblin_point = $5,
                updated_at = now()
            WHERE account_login = $1
            """,
            conn);
        cmd.Parameters.AddWithValue(login);
        cmd.Parameters.AddWithValue(row.WarehouseZen);
        cmd.Parameters.AddWithValue(row.WCoinC);
        cmd.Parameters.AddWithValue(row.WCoinP);
        cmd.Parameters.AddWithValue(row.GoblinPoint);
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
    }

    public ValueTask DisposeAsync() => _dataSource.DisposeAsync();

    static string NormaliseLogin(string accountLogin) =>
        accountLogin.Trim().ToLowerInvariant();
}
