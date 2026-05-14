using Npgsql;
using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

/// <summary>Opt-in: set <c>TEST_PG_CONNECTION_STRING</c> (libpq URI or Npgsql key=value) to validate local Docker Postgres.</summary>
public sealed class PostgresEnvSmokeTests
{
    [Fact]
    public async Task When_TEST_PG_CONNECTION_STRING_set_opens_connection()
    {
        var cs = Environment.GetEnvironmentVariable("TEST_PG_CONNECTION_STRING")?.Trim();
        if (string.IsNullOrEmpty(cs))
        {
            return;
        }

        await using var conn = new NpgsqlConnection(cs);
        await conn.OpenAsync();
        await using var cmd = new NpgsqlCommand("SELECT 1", conn);
        var one = await cmd.ExecuteScalarAsync();
        Assert.Equal(1, Convert.ToInt32(one, System.Globalization.CultureInfo.InvariantCulture));
    }

    [Fact]
    public void Character_roster_mirror_writer_no_repo_does_not_throw()
    {
        CharacterRosterMirrorWriter.ScheduleReplaceAccountRoster(
            "nope",
            Array.Empty<CharacterRosterRow>());
        Assert.Equal(0, CharacterRosterMirrorWriter.PendingUpsertsForTests);
    }
}
