using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CharacterRosterPostgresVitalsTests
{
    [Fact]
    public async Task Vitals_round_trip_when_TEST_PG_set()
    {
        var cs = Environment.GetEnvironmentVariable("TEST_PG_CONNECTION_STRING")?.Trim();
        if (string.IsNullOrEmpty(cs))
        {
            return;
        }

        var account = "m7vit_" + Guid.NewGuid().ToString("N")[..8];
        await using var repo = new PostgresCharacterRosterRepository(cs);
        var rows = new[]
        {
            CharacterRosterRowMapping.ToRow("VitHero", 0x20, 12, 0, 100, 110, 2, 250, 400, 90, 150, 42_000, 40, 200),
        };

        await repo.ReplaceAccountRosterAsync(account, rows);
        var loaded = await repo.LoadByAccountAsync(account);
        Assert.Single(loaded);
        Assert.Equal(250, loaded[0].CurrentHp);
        Assert.Equal(400, loaded[0].MaxHp);
        Assert.Equal(90, loaded[0].CurrentMp);
        Assert.Equal(150, loaded[0].MaxMp);
        Assert.Equal(42_000, loaded[0].Zen);
        Assert.Equal(40, loaded[0].CurrentShield);
        Assert.Equal(200, loaded[0].MaxShield);

        await repo.DeleteCharacterAsync(account, "VitHero");
        await repo.ReplaceAccountRosterAsync(account, Array.Empty<CharacterRosterRow>());
    }
}
