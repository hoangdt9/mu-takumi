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

    [Fact]
    public void TryValidateBlockKey_accepts_server_seed_chain()
    {
        var session = Guid.NewGuid();
        var seed = 99u;
        MoveMapSessionState.Reset(session, seed);
        var key = MoveMapKeyGenerator.GenerateKeyValue(seed);

        Assert.True(MoveMapKeyGenerator.TryValidateBlockKey(session, key, out var legacy));
        Assert.False(legacy);
        Assert.True(MoveMapSessionState.TryGet(session, out var next));
        Assert.Equal(key, next);
        MoveMapSessionState.Remove(session);
    }

    [Fact]
    public void TryValidateBlockKey_falls_back_to_client_seed_zero()
    {
        var session = Guid.NewGuid();
        MoveMapSessionState.Reset(session, 424242u);
        var legacyKey = MoveMapKeyGenerator.GenerateKeyValue(0);

        Assert.True(MoveMapKeyGenerator.TryValidateBlockKey(session, legacyKey, out var legacy));
        Assert.True(legacy);
        Assert.True(MoveMapSessionState.TryGet(session, out var next));
        Assert.Equal(legacyKey, next);
        MoveMapSessionState.Remove(session);
    }

    [Fact]
    public void TryValidateBlockKey_accepts_when_no_session_seed()
    {
        var session = Guid.NewGuid();
        var key = MoveMapKeyGenerator.GenerateKeyValue(0);

        Assert.True(MoveMapKeyGenerator.TryValidateBlockKey(session, key, out var legacy));
        Assert.True(legacy);
        MoveMapSessionState.Remove(session);
    }
}
