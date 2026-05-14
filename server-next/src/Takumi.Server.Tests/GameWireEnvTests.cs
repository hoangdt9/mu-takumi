using Takumi.Server.Game;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class GameWireEnvTests
{
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
}
