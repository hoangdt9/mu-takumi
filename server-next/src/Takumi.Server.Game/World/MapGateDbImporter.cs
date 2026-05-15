using Takumi.Server.Persistence;

namespace Takumi.Server.Game.World;

public static class MapGateDbImporter
{
    public static async Task<int> ImportFileAsync(string gatePath, CancellationToken ct = default)
    {
        var cs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv()
                 ?? throw new InvalidOperationException(
                     "No Postgres connection: set TAKUMI_PG_CONNECTION_STRING or TAKUMI_PG_HOST.");

        var entries = GateLoader.LoadFromFile(gatePath);
        var rows = entries.Select(MapGateRowMapping.ToRow).ToList();
        await using var repo = new PostgresMapGateRepository(cs);
        await repo.ReplaceAllAsync(rows, Path.GetFileName(gatePath), ct).ConfigureAwait(false);
        return rows.Count;
    }

    public static Task<int> ImportFromDataRootAsync(string? dataRoot = null, CancellationToken ct = default)
    {
        var root = WorldDataPathResolver.ResolveOrThrow(dataRoot);
        return ImportFileAsync(Path.Combine(root, "Move", "Gate.txt"), ct);
    }
}
