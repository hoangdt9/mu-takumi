using Takumi.Server.Game.World;
using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterSpawnPostgresEtlTests
{
    [Fact]
    public async Task Round_trip_replace_all_when_TEST_PG_set()
    {
        var cs = Environment.GetEnvironmentVariable("TEST_PG_CONNECTION_STRING")?.Trim();
        if (string.IsNullOrEmpty(cs))
        {
            return;
        }

        var rows = new[]
        {
            new MonsterSpawnRow
            {
                SpawnType = 0,
                MonsterClass = 3,
                MapId = 0,
                Dis = 0,
                PosX = 180,
                PosY = 120,
                Dir = 3,
            },
            new MonsterSpawnRow
            {
                SpawnType = 1,
                MonsterClass = 2,
                MapId = 0,
                Dis = 0,
                PosX = 140,
                PosY = 90,
                RangeTx = 150,
                RangeTy = 100,
                Dir = 1,
            },
        };

        await using var repo = new PostgresMonsterSpawnRepository(cs);
        await repo.ReplaceAllAsync(rows, "test-fixture.txt");

        var loaded = await repo.LoadAllAsync();
        Assert.Equal(2, loaded.Count);
        Assert.Equal(3, loaded[0].MonsterClass);
        Assert.Equal(2, loaded[1].MonsterClass);
        Assert.Equal(150, loaded[1].RangeTx);

        var entry = MonsterSpawnRowMapping.FromRow(loaded[0]);
        Assert.Equal(0, entry.SpawnType);
        Assert.Equal((byte)0, entry.Map);
        Assert.Equal(180, entry.X);

        await repo.ReplaceAllAsync(Array.Empty<MonsterSpawnRow>(), null);
    }

    [Fact]
    public async Task Import_file_etl_when_TEST_PG_and_set_base_path_set()
    {
        var cs = Environment.GetEnvironmentVariable("TEST_PG_CONNECTION_STRING")?.Trim();
        var path = Environment.GetEnvironmentVariable("TAKUMI_MONSTER_SET_BASE_PATH")?.Trim();
        if (string.IsNullOrEmpty(cs) || string.IsNullOrEmpty(path) || !File.Exists(path))
        {
            return;
        }

        Environment.SetEnvironmentVariable("TAKUMI_PG_CONNECTION_STRING", cs);
        var count = await MonsterSpawnDbImporter.ImportFileAsync(path);
        Assert.True(count > 0);

        var verifyCs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv()
                       ?? throw new InvalidOperationException("TAKUMI_PG_CONNECTION_STRING not set after import.");
        await using var repo = new PostgresMonsterSpawnRepository(verifyCs);
        var loaded = await repo.LoadAllAsync();
        Assert.Equal(count, loaded.Count);
    }
}
