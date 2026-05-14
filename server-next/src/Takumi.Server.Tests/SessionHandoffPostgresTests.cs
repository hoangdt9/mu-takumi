using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class SessionHandoffPostgresTests
{
    [Fact]
    public async Task ReplacePending_then_consume_once_when_TEST_PG_configured()
    {
        var cs = Environment.GetEnvironmentVariable("TEST_PG_CONNECTION_STRING")?.Trim();
        if (string.IsNullOrEmpty(cs))
        {
            return;
        }

        await using var repo = new PostgresSessionHandoffRepository(cs);
        var acct = "handoff_" + Guid.NewGuid().ToString("N")[..10];
        var tid = Guid.NewGuid();
        var exp = DateTimeOffset.UtcNow.AddHours(2);
        await repo.ReplacePendingForAccountAsync(acct, tid, exp, "127.0.0.1");
        Assert.True(await repo.TryConsumePendingAsync(acct, "127.0.0.1", matchClientIp: true));
        Assert.False(await repo.TryConsumePendingAsync(acct, "127.0.0.1", matchClientIp: true));
    }

    [Fact]
    public async Task Consume_fails_when_ip_mismatch_and_match_required()
    {
        var cs = Environment.GetEnvironmentVariable("TEST_PG_CONNECTION_STRING")?.Trim();
        if (string.IsNullOrEmpty(cs))
        {
            return;
        }

        await using var repo = new PostgresSessionHandoffRepository(cs);
        var acct = "handoff_" + Guid.NewGuid().ToString("N")[..10];
        var tid = Guid.NewGuid();
        await repo.ReplacePendingForAccountAsync(acct, tid, DateTimeOffset.UtcNow.AddHours(1), "10.0.0.1");
        Assert.False(await repo.TryConsumePendingAsync(acct, "10.0.0.2", matchClientIp: true));
        Assert.True(await repo.TryConsumePendingAsync(acct, "10.0.0.2", matchClientIp: false));
    }
}
