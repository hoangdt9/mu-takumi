using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class AccountPasswordHasherTests
{
    [Fact]
    public void Hash_and_verify_roundtrip()
    {
        var hash = AccountPasswordHasher.Hash("secret");
        Assert.True(AccountPasswordHasher.Verify("secret", hash));
        Assert.False(AccountPasswordHasher.Verify("wrong", hash));
    }

    [Fact]
    public void Dev_seed_test_account_hash_matches_plain_test()
    {
        const string dbHash = "$2a$11$grND7s6EmxqEhu8wrhBQZekPN..8eMVsvXAH7/ixhEsCSBdsqbNrO";
        Assert.True(AccountPasswordHasher.Verify("test", dbHash));
        Assert.False(AccountPasswordHasher.Verify("trst", dbHash));
    }
}
