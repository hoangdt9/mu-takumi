using Npgsql;

namespace Takumi.Server.Persistence;

/// <summary>Runtime account store in <c>public.account</c> (login + in-game register).</summary>
public sealed class PostgresAccountRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresAccountRepository(string connectionString) =>
        this._dataSource = NpgsqlDataSource.Create(connectionString);

    public async Task<bool> ExistsAsync(string accountLogin, CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            "SELECT 1 FROM account WHERE account_login = @login LIMIT 1",
            conn);
        cmd.Parameters.AddWithValue("login", NormaliseLogin(accountLogin));
        return await cmd.ExecuteScalarAsync(ct).ConfigureAwait(false) is not null;
    }

    public async Task<bool> TryCreateAsync(
        string accountLogin,
        string plainPassword,
        string securityCode,
        string phone,
        CancellationToken ct = default)
    {
        var login = NormaliseLogin(accountLogin);
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            INSERT INTO account (account_login, password_hash, security_code, phone)
            VALUES (@login, @hash, @sec, @phone)
            ON CONFLICT (account_login) DO NOTHING
            """,
            conn);
        cmd.Parameters.AddWithValue("login", login);
        cmd.Parameters.AddWithValue("hash", AccountPasswordHasher.Hash(plainPassword));
        cmd.Parameters.AddWithValue("sec", securityCode ?? string.Empty);
        cmd.Parameters.AddWithValue("phone", phone ?? string.Empty);
        return await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false) == 1;
    }

    public async Task<bool> VerifyPasswordAsync(string accountLogin, string plainPassword, CancellationToken ct = default)
    {
        var login = NormaliseLogin(accountLogin);
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            "SELECT password_hash FROM account WHERE account_login = @login LIMIT 1",
            conn);
        cmd.Parameters.AddWithValue("login", login);
        var hash = await cmd.ExecuteScalarAsync(ct).ConfigureAwait(false) as string;
        return hash is not null && AccountPasswordHasher.Verify(plainPassword, hash);
    }

    /// <summary>Password reset when security code + phone match (legacy JoinServer reset flow).</summary>
    public async Task<bool> TryResetPasswordAsync(
        string accountLogin,
        string securityCode,
        string phone,
        string newPlainPassword,
        CancellationToken ct = default)
    {
        var login = NormaliseLogin(accountLogin);
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            UPDATE account
            SET password_hash = @hash, updated_at = now()
            WHERE account_login = @login
              AND security_code = @sec
              AND phone = @phone
            """,
            conn);
        cmd.Parameters.AddWithValue("login", login);
        cmd.Parameters.AddWithValue("hash", AccountPasswordHasher.Hash(newPlainPassword));
        cmd.Parameters.AddWithValue("sec", securityCode ?? string.Empty);
        cmd.Parameters.AddWithValue("phone", phone ?? string.Empty);
        return await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false) == 1;
    }

    /// <summary>Upsert dev seed account when missing (plain password from env).</summary>
    public async Task SeedIfMissingAsync(string accountLogin, string plainPassword, CancellationToken ct = default)
    {
        if (await this.ExistsAsync(accountLogin, ct).ConfigureAwait(false))
        {
            return;
        }

        await this.TryCreateAsync(accountLogin, plainPassword, securityCode: string.Empty, phone: string.Empty, ct)
            .ConfigureAwait(false);
    }

    public async ValueTask DisposeAsync() => await this._dataSource.DisposeAsync().ConfigureAwait(false);

    internal static string NormaliseLogin(string accountLogin) =>
        accountLogin.Trim().ToLowerInvariant();
}
