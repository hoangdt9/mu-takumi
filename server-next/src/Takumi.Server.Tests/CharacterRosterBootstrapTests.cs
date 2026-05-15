using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CharacterRosterBootstrapTests
{
    [Fact]
    public void Db_primary_requires_db_sync_env()
    {
        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC", null);
        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_PRIMARY", "1");
        Assert.False(CharacterRosterBootstrap.IsDbPrimaryEnabled());

        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC", "1");
        Assert.True(CharacterRosterBootstrap.IsDbPrimaryEnabled());

        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_PRIMARY", null);
        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC", null);
    }

    [Fact]
    public void Skip_json_export_when_primary_unless_json_export_forced()
    {
        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC", "1");
        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_PRIMARY", "1");
        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_JSON_EXPORT", null);
        Assert.True(CharacterRosterBootstrap.ShouldSkipJsonExportOnSave());

        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_JSON_EXPORT", "1");
        Assert.False(CharacterRosterBootstrap.ShouldSkipJsonExportOnSave());

        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_PRIMARY", null);
        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC", null);
        Environment.SetEnvironmentVariable("TAKUMI_ROSTER_JSON_EXPORT", null);
    }
}
