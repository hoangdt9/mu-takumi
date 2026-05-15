using Takumi.Server.Persistence;

namespace Takumi.Server.Game.World;

public static class NpcShopDbImporter
{
    public static async Task<(int Shops, int Items)> ImportFromDataRootAsync(string? dataRoot = null, CancellationToken ct = default)
    {
        var cs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv()
                 ?? throw new InvalidOperationException(
                     "No Postgres connection: set TAKUMI_PG_CONNECTION_STRING or TAKUMI_PG_HOST.");

        var root = WorldDataPathResolver.ResolveOrThrow(dataRoot);
        var managerPath = Path.Combine(root, "ShopManager.txt");
        var shops = ShopManagerLoader.LoadFromFile(managerPath);
        var items = ShopItemLoader.LoadAllForDataRoot(root, shops);

        var shopRows = shops.Select(NpcShopRowMapping.ToRow).ToList();
        var itemRows = items.Select(NpcShopRowMapping.ToRow).ToList();

        await using var repo = new PostgresNpcShopRepository(cs);
        await repo.ReplaceAllAsync(shopRows, itemRows, "ShopManager.txt", ct).ConfigureAwait(false);
        return (shopRows.Count, itemRows.Count);
    }
}
