using Takumi.Server.Persistence;

namespace Takumi.Server.Game.World;

/// <summary>M8: NPC shop index + items (DB or files). Used for future <c>0x31</c> shop list parity.</summary>
public static class NpcShopCatalog
{
    static readonly object InitLock = new();
    static bool _initialized;
    static Dictionary<int, NpcShopEntry> _shops = new();
    static Dictionary<int, List<NpcShopItemEntry>> _itemsByShop = new();

    public static void EnsureInitialized()
    {
        if (_initialized)
        {
            return;
        }

        lock (InitLock)
        {
            if (_initialized)
            {
                return;
            }

            if (TryLoadFromPostgres())
            {
                Console.WriteLine(
                    "[m8] NpcShopCatalog: {0} shops, {1} items from Postgres",
                    _shops.Count,
                    _itemsByShop.Values.Sum(v => v.Count));
            }
            else
            {
                LoadFromFiles();
            }

            _initialized = true;
        }
    }

    public static int ResolveShopIndex(int monsterClass, byte mapId, byte x, byte y)
    {
        EnsureInitialized();
        var bestScore = -1;
        var bestIndex = -1;
        foreach (var s in _shops.Values)
        {
            if (s.MonsterClass != monsterClass)
            {
                continue;
            }

            if (s.MapId is not null && s.MapId.Value != mapId)
            {
                continue;
            }

            if (s.PosX is not null && s.PosX.Value != x)
            {
                continue;
            }

            if (s.PosY is not null && s.PosY.Value != y)
            {
                continue;
            }

            var score = 0;
            if (s.MapId is not null)
            {
                score += 4;
            }

            if (s.PosX is not null)
            {
                score += 2;
            }

            if (s.PosY is not null)
            {
                score += 1;
            }

            if (score > bestScore)
            {
                bestScore = score;
                bestIndex = s.ShopIndex;
            }
        }

        return bestIndex;
    }

    public static void LoadForTests(IReadOnlyList<NpcShopEntry> shops, IReadOnlyList<NpcShopItemEntry> items)
    {
        lock (InitLock)
        {
            _shops = shops.ToDictionary(s => s.ShopIndex);
            _itemsByShop = items.GroupBy(i => i.ShopIndex).ToDictionary(g => g.Key, g => g.ToList());
            _initialized = true;
        }
    }

    public static IReadOnlyList<NpcShopItemEntry> GetItems(int shopIndex)
    {
        EnsureInitialized();
        return _itemsByShop.TryGetValue(shopIndex, out var list) ? list : Array.Empty<NpcShopItemEntry>();
    }

    static bool TryLoadFromPostgres()
    {
        var repo = TakumiPostgresMirror.NpcShop;
        if (repo is null)
        {
            return false;
        }

        try
        {
            var (shopRows, itemRows) = repo.LoadAllAsync().GetAwaiter().GetResult();
            if (shopRows.Count == 0)
            {
                return false;
            }

            _shops = shopRows.Select(NpcShopRowMapping.FromRow).ToDictionary(s => s.ShopIndex);
            _itemsByShop = itemRows
                .Select(NpcShopRowMapping.FromRow)
                .GroupBy(i => i.ShopIndex)
                .ToDictionary(g => g.Key, g => g.ToList());
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine("[m8] npc_shop load failed: {0}", ex.Message);
            return false;
        }
    }

    static void LoadFromFiles()
    {
        var root = WorldDataPathResolver.ResolveDataRoot();
        if (root is null)
        {
            Console.WriteLine("[m8] NpcShopCatalog: no Data root");
            return;
        }

        var manager = Path.Combine(root, "ShopManager.txt");
        if (!File.Exists(manager))
        {
            Console.WriteLine("[m8] NpcShopCatalog: missing ShopManager.txt");
            return;
        }

        var shops = ShopManagerLoader.LoadFromFile(manager);
        var items = ShopItemLoader.LoadAllForDataRoot(root, shops);
        _shops = shops.ToDictionary(s => s.ShopIndex);
        _itemsByShop = items.GroupBy(i => i.ShopIndex).ToDictionary(g => g.Key, g => g.ToList());
        Console.WriteLine(
            "[m8] NpcShopCatalog: {0} shops, {1} items from files",
            _shops.Count,
            items.Count);
    }
}
