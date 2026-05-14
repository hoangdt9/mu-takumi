using System.Collections.Concurrent;

namespace Takumi.Server.Join;

/// <summary>In-process join / game handoff credential (M5). Dedicated <see cref="Takumi.Server.Game"/> (M6) can validate by <see cref="SessionTicket.TicketId"/>.</summary>
public readonly record struct SessionTicket(Guid TicketId, string AccountId, DateTimeOffset ExpiresUtc);

/// <summary>Thread-safe ticket table: one active ticket per account; re-login replaces the previous ticket.</summary>
public sealed class InMemorySessionTicketStore
{
    private readonly record struct Entry(string AccountId, DateTimeOffset ExpiresUtc);

    private readonly ConcurrentDictionary<Guid, Entry> _tickets = new();
    private readonly ConcurrentDictionary<string, Guid> _latestTicketByAccount = new();

    public SessionTicket Issue(string accountId, TimeSpan ttl)
    {
        ArgumentException.ThrowIfNullOrEmpty(accountId);
        if (ttl <= TimeSpan.Zero)
        {
            throw new ArgumentOutOfRangeException(nameof(ttl), ttl, "TTL must be positive.");
        }

        EvictLatestForAccount(accountId);

        var id = Guid.NewGuid();
        var exp = DateTimeOffset.UtcNow.Add(ttl);
        _tickets[id] = new Entry(accountId, exp);
        _latestTicketByAccount[accountId] = id;
        return new SessionTicket(id, accountId, exp);
    }

    /// <summary>Extend expiry from <b>now</b> when the account already entered the world (e.g. after <c>F3 03</c>).</summary>
    public void Touch(Guid ticketId, TimeSpan ttl)
    {
        if (ttl <= TimeSpan.Zero)
        {
            return;
        }

        if (!_tickets.TryGetValue(ticketId, out var e))
        {
            return;
        }

        if (e.ExpiresUtc <= DateTimeOffset.UtcNow)
        {
            RemoveTicket(ticketId, e.AccountId);
            return;
        }

        var newExp = DateTimeOffset.UtcNow.Add(ttl);
        if (newExp <= e.ExpiresUtc)
        {
            return;
        }

        _tickets[ticketId] = new Entry(e.AccountId, newExp);
    }

    /// <summary>Validate ticket for a second TCP (M6): returns account when non-expired.</summary>
    public bool TryValidate(Guid ticketId, out string accountId, out DateTimeOffset expiresUtc)
    {
        accountId = string.Empty;
        expiresUtc = default;
        if (!_tickets.TryGetValue(ticketId, out var e))
        {
            return false;
        }

        if (e.ExpiresUtc <= DateTimeOffset.UtcNow)
        {
            RemoveTicket(ticketId, e.AccountId);
            return false;
        }

        accountId = e.AccountId;
        expiresUtc = e.ExpiresUtc;
        return true;
    }

    /// <summary>Optional stricter check: ticket must belong to <paramref name="expectedAccountId"/>.</summary>
    public bool TryValidate(Guid ticketId, string expectedAccountId, out DateTimeOffset expiresUtc)
    {
        if (!TryValidate(ticketId, out var acct, out expiresUtc))
        {
            return false;
        }

        if (!string.Equals(acct, expectedAccountId, StringComparison.Ordinal))
        {
            return false;
        }

        return true;
    }

    public void RevokeTicket(Guid ticketId)
    {
        if (_tickets.TryRemove(ticketId, out var e))
        {
            if (_latestTicketByAccount.TryGetValue(e.AccountId, out var lid) && lid == ticketId)
            {
                _latestTicketByAccount.TryRemove(e.AccountId, out _);
            }
        }
    }

    private void EvictLatestForAccount(string accountId)
    {
        if (_latestTicketByAccount.TryRemove(accountId, out var previous))
        {
            _tickets.TryRemove(previous, out _);
        }
    }

    private void RemoveTicket(Guid ticketId, string accountId)
    {
        _tickets.TryRemove(ticketId, out _);
        if (_latestTicketByAccount.TryGetValue(accountId, out var lid) && lid == ticketId)
        {
            _latestTicketByAccount.TryRemove(accountId, out _);
        }
    }
}
