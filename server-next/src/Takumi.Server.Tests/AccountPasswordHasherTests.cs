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
}
