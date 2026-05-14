using Takumi.Server.Persistence;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class JoinInventoryPacket602Tests
{
    [Fact]
    public async Task Build_without_repo_returns_empty_F3_10()
    {
        var name = new byte[10];
        System.Text.Encoding.ASCII.GetBytes("HERO").CopyTo(name, 0);
        var pkt = await JoinInventoryPacket602.BuildAsync(null, "admin", name, CancellationToken.None);
        Assert.Equal(InventoryListWire602.BuildEmpty(), pkt);
    }

    [Fact]
    public async Task Build_with_null_or_whitespace_account_returns_empty()
    {
        var pkt = await JoinInventoryPacket602.BuildAsync(null, "", new byte[10], CancellationToken.None);
        Assert.Equal(InventoryListWire602.BuildEmpty(), pkt);
    }
}
