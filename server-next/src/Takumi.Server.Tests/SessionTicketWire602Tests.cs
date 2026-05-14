using System.Text;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class SessionTicketWire602Tests
{
    [Fact]
    public void Push_packet_round_trips_attach_finder_and_body()
    {
        var tid = Guid.Parse("a1a2a3a4-b1b2-c1c2-d1d2-d3d4d5d6d7d8");
        const long expUnix = 1_700_000_000L;
        Span<byte> acct10 = stackalloc byte[SessionTicketSignature602.AccountWireBytes];
        SessionTicketSignature602.FormatAccount10("testacc", acct10);
        var key = Encoding.UTF8.GetBytes("test-secret-8b");
        var mac = SessionTicketSignature602.ComputeMacV1(key, tid, expUnix, acct10);
        var push = SessionTicketWire602.BuildPushC1(tid, expUnix, acct10, mac);
        Assert.Equal(SessionTicketWire602.PushPacketTotalLength, push.Length);
        Assert.Equal(SessionTicketWire602.ServerPushSubCode, push[3]);
        var body = push.AsSpan(4, SessionTicketWire602.BodyBytes);
        Assert.True(SessionTicketWire602.TryReadBody(body, out var tid2, out var exp2, out var a2, out var m2));
        Assert.Equal(tid, tid2);
        Assert.Equal(expUnix, exp2);
        Assert.True(a2.SequenceEqual(acct10));
        Assert.True(m2.SequenceEqual(mac));

        var attach = (byte[])push.Clone();
        attach[3] = SessionTicketWire602.ClientAttachSubCode;
        Assert.True(SessionTicketWire602.TryFindClientAttach(attach.AsSpan(), out var bodyViaFinder));
        Assert.True(bodyViaFinder.SequenceEqual(body));
    }
}
