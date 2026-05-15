using Takumi.Server.Persistence;

namespace Takumi.Server.Game.World;

/// <summary>M8 ETL: <c>MonsterSetBase.txt</c> → <c>monster_spawn</c> Postgres table.</summary>
public static class MonsterSpawnDbImporter
{
    public static async Task<int> ImportFileAsync(string filePath, CancellationToken ct = default)
    {
        var cs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv()
                 ?? throw new InvalidOperationException(
                     "No Postgres connection: set TAKUMI_PG_CONNECTION_STRING or TAKUMI_PG_HOST.");

        var entries = MonsterSetBaseLoader.LoadFromFile(filePath);
        var rows = entries.Select(MonsterSpawnRowMapping.ToRow).ToList();
        await using var repo = new PostgresMonsterSpawnRepository(cs);
        await repo.ReplaceAllAsync(rows, Path.GetFileName(filePath), ct).ConfigureAwait(false);
        return rows.Count;
    }
}
