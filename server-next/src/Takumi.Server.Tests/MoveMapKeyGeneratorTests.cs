using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MoveMapKeyGeneratorTests
{
    [Fact]
    public void GenerateKeyValue_MatchesClientSeedZero()
    {
        Assert.Equal(37531607u, MoveMapKeyGenerator.GenerateKeyValue(0));
    }

    [Fact]
    public void TryAcceptKey_AdvancesSeed()
    {
        var seed = 42u;
        var next = MoveMapKeyGenerator.GenerateKeyValue(seed);
        Assert.True(MoveMapKeyGenerator.TryAcceptKey(ref seed, next));
        Assert.Equal(next, seed);
    }

    [Fact]
    public void BuildChecksum_HasExpectedLayout()
    {
        var packet = MoveMapWire602.BuildChecksum(0xAABBCCDD);
        Assert.Equal(new byte[] { 0xC1, 0x08, 0x8E, 0x01, 0xDD, 0xCC, 0xBB, 0xAA }, packet);
    }
}
