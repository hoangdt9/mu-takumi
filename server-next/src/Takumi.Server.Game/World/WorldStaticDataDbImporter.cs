namespace Takumi.Server.Game.World;

/// <summary>Runs all M8 ETL importers against a GameServer <c>Data</c> root.</summary>
public static class WorldStaticDataDbImporter
{
    public static async Task<(int Gates, int Shops, int ShopItems, int CustomFiles)> ImportAllAsync(
        string? dataRoot = null,
        CancellationToken ct = default)
    {
        var gates = await MapGateDbImporter.ImportFromDataRootAsync(dataRoot, ct).ConfigureAwait(false);
        var (shops, items) = await NpcShopDbImporter.ImportFromDataRootAsync(dataRoot, ct).ConfigureAwait(false);
        var customDir = dataRoot is null
            ? Path.Combine(WorldDataPathResolver.ResolveOrThrow(), "Custom")
            : Path.Combine(dataRoot, "Custom");
        var custom = Directory.Exists(customDir)
            ? await CustomWorldConfigDbImporter.ImportDirectoryAsync(customDir, ct).ConfigureAwait(false)
            : 0;
        return (gates, shops, items, custom);
    }
}
