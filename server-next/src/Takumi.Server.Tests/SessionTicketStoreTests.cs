using Takumi.Server.Join;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class SessionTicketStoreTests
{
    [Fact]
    public void Issue_then_TryValidate_returns_account()
    {
        var s = new InMemorySessionTicketStore();
        var t = s.Issue("admin", TimeSpan.FromMinutes(10));
        Assert.True(s.TryValidate(t.TicketId, out var acct, out var exp));
        Assert.Equal("admin", acct);
        Assert.True(exp > DateTimeOffset.UtcNow);
    }

    [Fact]
    public void Second_login_for_same_account_replaces_ticket()
    {
        var s = new InMemorySessionTicketStore();
        var first = s.Issue("u1", TimeSpan.FromHours(1));
        var second = s.Issue("u1", TimeSpan.FromHours(1));
        Assert.False(s.TryValidate(first.TicketId, out _, out _));
        Assert.True(s.TryValidate(second.TicketId, out var acct, out _));
        Assert.Equal("u1", acct);
    }

    [Fact]
    public void TryValidate_with_wrong_account_fails()
    {
        var s = new InMemorySessionTicketStore();
        var t = s.Issue("a", TimeSpan.FromHours(1));
        Assert.False(s.TryValidate(t.TicketId, "b", out _));
    }

    [Fact]
    public void Expired_ticket_is_rejected()
    {
        var s = new InMemorySessionTicketStore();
        var t = s.Issue("x", TimeSpan.FromMilliseconds(20));
        Thread.Sleep(200);
        Assert.False(s.TryValidate(t.TicketId, out _, out _));
    }

    [Fact]
    public void Touch_extends_expiry_from_now()
    {
        var s = new InMemorySessionTicketStore();
        var t = s.Issue("x", TimeSpan.FromMilliseconds(40));
        Thread.Sleep(25);
        s.Touch(t.TicketId, TimeSpan.FromMinutes(5));
        Assert.True(s.TryValidate(t.TicketId, out _, out var exp));
        Assert.True(exp > DateTimeOffset.UtcNow.AddMinutes(4));
    }

    [Fact]
    public void RevokeTicket_removes_entry()
    {
        var s = new InMemorySessionTicketStore();
        var t = s.Issue("z", TimeSpan.FromHours(1));
        s.RevokeTicket(t.TicketId);
        Assert.False(s.TryValidate(t.TicketId, out _, out _));
    }
}
