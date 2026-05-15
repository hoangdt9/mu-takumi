using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CharacterDomainMirrorTests
{
    [Fact]
    public void IsEnabled_requires_db_sync_and_flag()
    {
        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC", null);
        Environment.SetEnvironmentVariable("TAKUMI_CHARACTER_DOMAIN_SYNC", "1");
        Assert.False(CharacterDomainMirrorWriter.IsEnabled());

        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC", "1");
        Assert.True(CharacterDomainMirrorWriter.IsEnabled());

        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC", null);
        Environment.SetEnvironmentVariable("TAKUMI_CHARACTER_DOMAIN_SYNC", null);
    }
}
