using Npgsql;
using NpgsqlTypes;
using Takumi.Server.Protocol;

namespace Takumi.Server.Persistence;

/// <summary>
/// Persists join handoff rows for split-stack QA: login host issues after F1 01 ok; game host may require
/// <see cref="TryConsumePendingAsync"/> before accepting the same account on <c>TAKUMI_GAME_PORT</c>.
/// </summary>
public sealed class PostgresSessionHandoffRepository : IAsyncDisposable
{
    private readonly NpgsqlDataSource _dataSource;

    public PostgresSessionHandoffRepository(string connectionString) =>
        this._dataSource = NpgsqlDataSource.Create(connectionString);

    /// <summary>Replace any pending ticket for the account, then insert the new row (matches in-memory "one ticket per account").</summary>
    public async Task ReplacePendingForAccountAsync(
        string accountLogin,
        Guid ticketId,
        DateTimeOffset expiresUtc,
        string? clientIp,
        CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var tx = await conn.BeginTransactionAsync(ct).ConfigureAwait(false);
        await using (var del = new NpgsqlCommand(
                         """
                         DELETE FROM session_ticket
                         WHERE account_login = $1 AND consumed_utc IS NULL
                         """,
                         conn,
                         tx))
        {
            del.Parameters.Add(new NpgsqlParameter("a", NpgsqlDbType.Text) { Value = accountLogin });
            await del.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        await using (var ins = new NpgsqlCommand(
                         """
                         INSERT INTO session_ticket (ticket_id, account_login, expires_utc, client_ip)
                         VALUES ($1, $2, $3, $4)
                         """,
                         conn,
                         tx))
        {
            ins.Parameters.Add(new NpgsqlParameter("id", NpgsqlDbType.Uuid) { Value = ticketId });
            ins.Parameters.Add(new NpgsqlParameter("a", NpgsqlDbType.Text) { Value = accountLogin });
            ins.Parameters.Add(new NpgsqlParameter("e", NpgsqlDbType.TimestampTz) { Value = expiresUtc });
            ins.Parameters.Add(new NpgsqlParameter("ip", NpgsqlDbType.Text) { Value = (object?)clientIp ?? DBNull.Value });
            await ins.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
    }

    public async Task TouchExpiresAsync(Guid ticketId, DateTimeOffset newExpiresUtc, CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            UPDATE session_ticket
            SET expires_utc = $2
            WHERE ticket_id = $1 AND consumed_utc IS NULL AND expires_utc > CURRENT_TIMESTAMP
            """,
            conn);
        cmd.Parameters.Add(new NpgsqlParameter("id", NpgsqlDbType.Uuid) { Value = ticketId });
        cmd.Parameters.Add(new NpgsqlParameter("e", NpgsqlDbType.TimestampTz) { Value = newExpiresUtc });
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
    }

    public async Task DeleteByTicketAsync(Guid ticketId, CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand("DELETE FROM session_ticket WHERE ticket_id = $1", conn);
        cmd.Parameters.Add(new NpgsqlParameter("id", NpgsqlDbType.Uuid) { Value = ticketId });
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
    }

    /// <summary>
    /// Marks the newest pending ticket for <paramref name="accountLogin"/> as consumed. Returns false when none match
    /// (expired, wrong IP when <paramref name="matchClientIp"/> is true, or never issued from login host).
    /// </summary>
    public async Task<bool> TryConsumePendingAsync(
        string accountLogin,
        string? clientIp,
        bool matchClientIp,
        CancellationToken ct = default)
    {
        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var cmd = new NpgsqlCommand(
            """
            WITH pick AS (
                SELECT ticket_id
                FROM session_ticket
                WHERE account_login = $1
                  AND consumed_utc IS NULL
                  AND expires_utc > CURRENT_TIMESTAMP
                  AND ($2::boolean = false OR client_ip IS NOT DISTINCT FROM $3)
                ORDER BY issued_utc DESC
                LIMIT 1
            )
            UPDATE session_ticket st
            SET consumed_utc = CURRENT_TIMESTAMP
            FROM pick
            WHERE st.ticket_id = pick.ticket_id
            RETURNING st.ticket_id
            """,
            conn);
        cmd.Parameters.Add(new NpgsqlParameter("a", NpgsqlDbType.Text) { Value = accountLogin });
        cmd.Parameters.Add(new NpgsqlParameter("match", NpgsqlDbType.Boolean) { Value = matchClientIp });
        cmd.Parameters.Add(new NpgsqlParameter("ip", NpgsqlDbType.Text) { Value = (object?)clientIp ?? DBNull.Value });
        var o = await cmd.ExecuteScalarAsync(ct).ConfigureAwait(false);
        return o is not null && o is not DBNull;
    }

    /// <summary>
    /// Validates HMAC + DB row, then consumes the ticket. Used when the client sends <c>F1 0xA6</c> attach on the game TCP.
    /// </summary>
    public async Task<bool> TryConsumeSignedWireAttachAsync(
        Guid ticketIdFromWire,
        string accountLoginFromWire,
        long expiresUnixUtcFromWire,
        ReadOnlyMemory<byte> mac32FromWire,
        ReadOnlyMemory<byte> hmacKey,
        CancellationToken ct = default)
    {
        var macSpan = mac32FromWire.Span;
        var keySpan = hmacKey.Span;
        if (keySpan.Length < 8 || macSpan.Length != SessionTicketSignature602.MacLength)
        {
            return false;
        }

        Span<byte> acct10 = stackalloc byte[SessionTicketSignature602.AccountWireBytes];
        SessionTicketSignature602.FormatAccount10(accountLoginFromWire, acct10);
        var expectedMac = SessionTicketSignature602.ComputeMacV1(keySpan, ticketIdFromWire, expiresUnixUtcFromWire, acct10);
        if (!SessionTicketSignature602.FixedTimeEquals(expectedMac, macSpan))
        {
            return false;
        }

        var wireNorm = CharacterRosterMerge.NormaliseName(accountLoginFromWire);
        var wireExp = DateTimeOffset.FromUnixTimeSeconds(expiresUnixUtcFromWire);

        await using var conn = await this._dataSource.OpenConnectionAsync(ct).ConfigureAwait(false);
        await using var tx = await conn.BeginTransactionAsync(ct).ConfigureAwait(false);
        string dbAccount;
        DateTimeOffset dbExpires;
        DateTimeOffset? consumed;
        await using (var sel = new NpgsqlCommand(
                         """
                         SELECT account_login, expires_utc, consumed_utc
                         FROM session_ticket
                         WHERE ticket_id = $1
                         FOR UPDATE
                         """,
                         conn,
                         tx))
        {
            sel.Parameters.Add(new NpgsqlParameter("id", NpgsqlDbType.Uuid) { Value = ticketIdFromWire });
            await using var reader = await sel.ExecuteReaderAsync(ct).ConfigureAwait(false);
            if (!await reader.ReadAsync(ct).ConfigureAwait(false))
            {
                await tx.RollbackAsync(ct).ConfigureAwait(false);
                return false;
            }

            dbAccount = reader.GetString(0);
            dbExpires = reader.GetFieldValue<DateTimeOffset>(1);
            consumed = reader.IsDBNull(2) ? null : reader.GetFieldValue<DateTimeOffset>(2);
        }

        if (consumed is not null)
        {
            await tx.RollbackAsync(ct).ConfigureAwait(false);
            return false;
        }

        if (dbExpires <= DateTimeOffset.UtcNow)
        {
            await tx.RollbackAsync(ct).ConfigureAwait(false);
            return false;
        }

        if (!string.Equals(CharacterRosterMerge.NormaliseName(dbAccount), wireNorm, StringComparison.Ordinal))
        {
            await tx.RollbackAsync(ct).ConfigureAwait(false);
            return false;
        }

        if (Math.Abs((dbExpires - wireExp).TotalSeconds) > 3)
        {
            await tx.RollbackAsync(ct).ConfigureAwait(false);
            return false;
        }

        await using (var upd = new NpgsqlCommand(
                         """
                         UPDATE session_ticket
                         SET consumed_utc = CURRENT_TIMESTAMP
                         WHERE ticket_id = $1 AND consumed_utc IS NULL
                         RETURNING ticket_id
                         """,
                         conn,
                         tx))
        {
            upd.Parameters.Add(new NpgsqlParameter("id", NpgsqlDbType.Uuid) { Value = ticketIdFromWire });
            var o = await upd.ExecuteScalarAsync(ct).ConfigureAwait(false);
            if (o is null || o is DBNull)
            {
                await tx.RollbackAsync(ct).ConfigureAwait(false);
                return false;
            }
        }

        await tx.CommitAsync(ct).ConfigureAwait(false);
        return true;
    }

    public async ValueTask DisposeAsync()
    {
        await this._dataSource.DisposeAsync().ConfigureAwait(false);
    }
}
