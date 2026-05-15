using Takumi.Server.Game.World;
using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class WorldStaticDataPostgresEtlTests
{
    [Fact]
    public async Task Map_gate_round_trip_when_TEST_PG_set()
    {
        var cs = Environment.GetEnvironmentVariable("TEST_PG_CONNECTION_STRING")?.Trim();
        if (string.IsNullOrEmpty(cs))
        {
            return;
        }

        var rows = new[]
        {
            new MapGateRow
            {
                GateIndex = 9001,
                Flag = 1,
                MapId = 0,
                PosX = 10,
                PosY = 20,
                RangeTx = 11,
                RangeTy = 21,
                TargetGate = 9002,
                Dir = 3,
                MinLevel = -1,
                MaxLevel = 400,
                MinReset = -1,
                MaxReset = -1,
                AccountLevel = 0,
            },
        };

        await using var repo = new PostgresMapGateRepository(cs);
        await repo.ReplaceAllAsync(rows, "test");
        var loaded = await repo.LoadAllAsync();
        Assert.Contains(loaded, r => r.GateIndex == 9001 && r.MinLevel == -1);
        await repo.ReplaceAllAsync(Array.Empty<MapGateRow>(), null);
    }

    [Fact]
    public async Task Npc_shop_round_trip_when_TEST_PG_set()
    {
        var cs = Environment.GetEnvironmentVariable("TEST_PG_CONNECTION_STRING")?.Trim();
        if (string.IsNullOrEmpty(cs))
        {
            return;
        }

        var shops = new[]
        {
            new NpcShopRow { ShopIndex = 8800, MonsterClass = 251, MapId = null, PosX = null, PosY = null },
        };
        var items = new[]
        {
            new NpcShopItemRow
            {
                ShopIndex = 8800,
                Slot = 0,
                ItemGroup = 0,
                ItemIndex = 20,
                ItemLevel = 9,
                Durability = 0,
                Skill = 1,
                Luck = 1,
                Option = 7,
                ExcOpt = 63,
            },
        };

        await using var repo = new PostgresNpcShopRepository(cs);
        await repo.ReplaceAllAsync(shops, items, "test");
        var (loadedShops, loadedItems) = await repo.LoadAllAsync();
        Assert.Contains(loadedShops, s => s.ShopIndex == 8800 && s.MapId is null);
        Assert.Contains(loadedItems, i => i.ShopIndex == 8800 && i.ItemIndex == 20);
        await repo.ReplaceAllAsync(Array.Empty<NpcShopRow>(), Array.Empty<NpcShopItemRow>(), null);
    }

    [Fact]
    public async Task Import_all_from_gameserver_data_when_TEST_PG_and_data_path_set()
    {
        var cs = Environment.GetEnvironmentVariable("TEST_PG_CONNECTION_STRING")?.Trim();
        if (string.IsNullOrEmpty(cs))
        {
            return;
        }

        var dataRoot = Environment.GetEnvironmentVariable("TAKUMI_GAMESERVER_DATA_PATH")?.Trim();
        if (string.IsNullOrEmpty(dataRoot) || !Directory.Exists(dataRoot))
        {
            dataRoot = WorldDataPathResolver.ResolveDataRoot();
        }

        if (string.IsNullOrEmpty(dataRoot))
        {
            return;
        }

        Environment.SetEnvironmentVariable("TAKUMI_PG_CONNECTION_STRING", cs);
        var result = await WorldStaticDataDbImporter.ImportAllAsync(dataRoot);
        Assert.True(result.Gates > 0);
        Assert.True(result.Shops > 0);
        Assert.True(result.ShopItems > 0);
    }
}
