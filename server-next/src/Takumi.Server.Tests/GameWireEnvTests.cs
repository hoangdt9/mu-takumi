using System.IO.Pipelines;
using Takumi.Server.Game;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class GameWireEnvTests
{
    [Fact]
    public async Task ClientProtectInboundPump_restores_plain_before_sm_layer()
    {
        var keys = TakumiClientProtectWire602.DeriveKeys("takumi12", System.Text.Encoding.ASCII.GetBytes("TbYehR2hFUPBKgZj"));
        var plain = new byte[] { 0xC3, 0x5A, 0xF1, 0x01, 0x11, 0x22, 0x33 };
        var onWire = (byte[])plain.Clone();
        TakumiClientProtectWire602.EncryptInPlace(onWire, keys.EncDecKey1, keys.EncDecKey2);

        var source = new Pipe();
        var dest = new Pipe();
        await source.Writer.WriteAsync(onWire);
        source.Writer.Complete();

        using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(2));
        var pump = TakumiClientProtectInboundPump.RunAsync(source.Reader, dest.Writer, keys.EncDecKey1, keys.EncDecKey2, cts.Token);
        await pump;

        var read = await dest.Reader.ReadAsync(cts.Token);
        var seq = read.Buffer;
        var outBuf = new byte[seq.Length];
        var pos = 0;
        foreach (var mem in seq)
        {
            mem.Span.CopyTo(outBuf.AsSpan(pos));
            pos += mem.Length;
        }

        dest.Reader.AdvanceTo(seq.End);
        Assert.Equal(plain, outBuf);
    }

    [Fact]
    public void ParseJoinVersion5_accepts_ascii10405()
    {
        var v = GameWireEnv.ParseJoinVersion5("10405");
        Assert.NotNull(v);
        Assert.Equal(5, v!.Length);
        Assert.Equal("10405", System.Text.Encoding.ASCII.GetString(v));
    }

    [Fact]
    public void ParseSerial16_accepts_16_ascii()
    {
        var s = GameWireEnv.ParseSerial16("TbYehR2hFUPBKgZj");
        Assert.NotNull(s);
        Assert.Equal(16, s!.Length);
    }

    [Fact]
    public void ParseAccounts_splits_pipe_pairs()
    {
        var d = GameWireEnv.ParseAccounts("a:b|c:d");
        Assert.NotNull(d);
        Assert.Equal("b", d!["a"]);
        Assert.Equal("d", d["c"]);
    }

    [Fact]
    public void ResolveClientProtectOutboundKeys_defaults_on_for_takumi_serial()
    {
        var s = GameWireEnv.ParseSerial16("TbYehR2hFUPBKgZj");
        var k = GameWireEnv.ResolveClientProtectOutboundKeys(null, null, s);
        Assert.NotNull(k);
    }

    [Fact]
    public void ResolveClientProtectOutboundKeys_off_when_env_0()
    {
        var s = GameWireEnv.ParseSerial16("TbYehR2hFUPBKgZj");
        Assert.Null(GameWireEnv.ResolveClientProtectOutboundKeys("0", null, s));
    }

    [Fact]
    public void TakumiClientProtectWire_round_trips_sample_join()
    {
        var keys = TakumiClientProtectWire602.DeriveKeys("takumi12", System.Text.Encoding.ASCII.GetBytes("TbYehR2hFUPBKgZj"));
        var plain = new byte[] { 0xC1, 12, 0xF1, 0x00, 1, 0, 0, 0x31, 0x30, 0x34, 0x30, 0x35 };
        var buf = (byte[])plain.Clone();
        TakumiClientProtectWire602.EncryptInPlace(buf, keys.EncDecKey1, keys.EncDecKey2);
        Assert.NotEqual(plain, buf);
        TakumiClientProtectWire602.DecryptInPlace(buf, keys.EncDecKey1, keys.EncDecKey2);
        Assert.Equal(plain, buf);
    }
}
