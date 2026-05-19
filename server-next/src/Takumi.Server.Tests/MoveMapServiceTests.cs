using Takumi.Server.Game;
using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MoveMapServiceTests
{
    static MoveMapPlayerContext Ctx(
        int level = 50,
        long zen = 50000,
        int reset = 0,
        int accountLevel = 0,
        byte pk = 0,
        byte gensFamily = 1,
        byte currentMap = 0,
        byte serverClass = 0,
        bool teleport = false) =>
        new(
            level,
            zen,
            reset,
            accountLevel,
            pk,
            gensFamily,
            currentMap,
            serverClass,
            Guid.NewGuid(),
            ShopWarehouseOrTradeOpen: false,
            IsDead: false,
            teleport);

    [Fact]
    public void TryResolve_lorencia_index_uses_gate_destination_not_map_id()
    {
        MapGateCatalog.EnsureInitialized();
        MoveMapCatalog.LoadForTests(
        [
            new MoveMapEntry { Index = 2, Money = 2000, MinLevel = 10, MaxLevel = 10000, Gate = 17 },
        ]);

        if (!MapGateCatalog.TryGetGate(17, out _))
        {
            return;
        }

        var ok = MoveMapService.TryResolve(2, Ctx(level: 50), previousMap: 0, out var dest, out var deny, out var zen);
        Assert.True(ok);
        Assert.Equal(MoveMapService.DenyReason.None, deny);
        Assert.Equal(2000, zen);
        Assert.NotEqual(2, dest.MapId);
        Assert.False(dest.MapChanged);
    }

    [Fact]
    public void TryResolve_noria_from_lorencia_sets_map_changed()
    {
        MapGateCatalog.EnsureInitialized();
        MoveMapCatalog.LoadForTests(
        [
            new MoveMapEntry { Index = 3, Money = 2000, MinLevel = 10, MaxLevel = 10000, Gate = 22 },
        ]);

        if (!MapGateCatalog.TryGetGate(22, out var gate) || gate.MapId == 0)
        {
            return;
        }

        var ok = MoveMapService.TryResolve(3, Ctx(level: 50), previousMap: 0, out var dest, out _, out _);
        Assert.True(ok);
        Assert.NotEqual(0, dest.MapId);
        Assert.True(dest.MapChanged);
    }

    [Fact]
    public void TryResolve_denies_when_teleport_already_in_progress()
    {
        MoveMapCatalog.LoadForTests(
        [
            new MoveMapEntry { Index = 2, Money = 0, MinLevel = 1, MaxLevel = 400, Gate = 17 },
        ]);

        var ok = MoveMapService.TryResolve(2, Ctx(teleport: true), previousMap: 0, out _, out var deny, out _);
        Assert.False(ok);
        Assert.Equal(MoveMapService.DenyReason.TeleportInProgress, deny);
        Assert.Equal(MoveMapWire602.ResultFailedTeleport, MoveMapService.ToWireResult(deny));
    }

    [Fact]
    public void TryResolve_denies_min_reset()
    {
        MoveMapCatalog.LoadForTests(
        [
            new MoveMapEntry { Index = 1, Money = 0, MaxLevel = 10000, MinReset = 5, Gate = 17 },
        ]);

        var ok = MoveMapService.TryResolve(1, Ctx(reset: 0), previousMap: 0, out _, out var deny, out _);
        Assert.False(ok);
        Assert.Equal(MoveMapService.DenyReason.ResetTooLow, deny);
    }

    [Fact]
    public void TryResolve_denies_pk_murderer()
    {
        MoveMapCatalog.LoadForTests(
        [
            new MoveMapEntry { Index = 1, Money = 0, MaxLevel = 10000, Gate = 17 },
        ]);

        var ok = MoveMapService.TryResolve(1, Ctx(pk: 5), previousMap: 0, out _, out var deny, out _);
        Assert.False(ok);
        Assert.Equal(MoveMapService.DenyReason.PkMurderer, deny);
    }

    [Fact]
    public void TryResolve_denies_shop_open()
    {
        MoveMapCatalog.LoadForTests(
        [
            new MoveMapEntry { Index = 1, Money = 0, MaxLevel = 10000, Gate = 17 },
        ]);

        var sid = Guid.NewGuid();
        PlayerShopSession.OpenShop(sid, 0);
        try
        {
            var ctx = new MoveMapPlayerContext(50, 1000, 0, 0, 0, 1, 0, 0, sid, true, false, false);
            var ok = MoveMapService.TryResolve(1, ctx, previousMap: 0, out _, out var deny, out _);
            Assert.False(ok);
            Assert.Equal(MoveMapService.DenyReason.UiBlocked, deny);
        }
        finally
        {
            PlayerShopSession.CloseShop(sid);
        }
    }

    [Fact]
    public void Catalog_duplicate_index_last_row_wins()
    {
        MoveMapCatalog.LoadForTests(
        [
            new MoveMapEntry { Index = 26, Money = 0, Gate = 118 },
            new MoveMapEntry { Index = 26, Money = 27000, MinLevel = 150, Gate = 118 },
        ]);

        Assert.True(MoveMapCatalog.TryGet(26, out var entry));
        Assert.Equal(27000, entry!.Money);
        Assert.Equal(150, entry.MinLevel);
    }

    [Fact]
    public void ToWireResult_maps_level_and_zen_denials()
    {
        Assert.Equal(MoveMapWire602.ResultNotEnoughLevel, MoveMapService.ToWireResult(MoveMapService.DenyReason.LevelTooLow));
        Assert.Equal(MoveMapWire602.ResultNotEnoughZen, MoveMapService.ToWireResult(MoveMapService.DenyReason.NotEnoughZen));
        Assert.Equal(MoveMapWire602.ResultMurderer, MoveMapService.ToWireResult(MoveMapService.DenyReason.PkMurderer));
        Assert.Equal(MoveMapWire602.ResultGensRequired, MoveMapService.ToWireResult(MoveMapService.DenyReason.GensRequired));
    }

    [Fact]
    public void TryResolve_denies_gens_battle_without_family()
    {
        MapManagerCatalog.LoadForTests([new MapManagerEntry { MapId = 63, GensBattle = 1 }]);
        MapGateCatalog.LoadForTests(
        [
            new MapGateEntry { GateIndex = 99, MapId = 63, PosX = 15, PosY = 15, RangeTx = 15, RangeTy = 15, Dir = 0, TargetGate = 0 },
        ]);
        MoveMapCatalog.LoadForTests(
        [
            new MoveMapEntry { Index = 1, Money = 0, MaxLevel = 10000, Gate = 99 },
        ]);

        var ok = MoveMapService.TryResolve(1, Ctx(gensFamily: 0), previousMap: 0, out _, out var deny, out _);
        Assert.False(ok);
        Assert.Equal(MoveMapService.DenyReason.GensRequired, deny);
    }

    [Fact]
    public void TryResolve_denies_custom_arena_without_matching_gate()
    {
        CustomArenaCatalog.LoadForTests(
        [
            new CustomArenaRuleEntry { Index = 0, StartGate = 450, MinLevel = 400 },
        ]);
        MapGateCatalog.LoadForTests(
        [
            new MapGateEntry
            {
                GateIndex = 17,
                MapId = MapManagerCatalog.MapSilent,
                PosX = 15,
                PosY = 15,
                RangeTx = 15,
                RangeTy = 15,
                Dir = 0,
                TargetGate = 0,
            },
        ]);
        MoveMapCatalog.LoadForTests(
        [
            new MoveMapEntry { Index = 1, Money = 0, MaxLevel = 10000, Gate = 17 },
        ]);

        var ok = MoveMapService.TryResolve(1, Ctx(level: 50), previousMap: 0, out _, out var deny, out _);
        Assert.False(ok);
        Assert.Equal(MoveMapService.DenyReason.CustomArenaBlocked, deny);
    }

    [Fact]
    public void MoveLoader_parses_move_txt_tail_columns()
    {
        var path = ResolveMovePath();
        if (!File.Exists(path))
        {
            return;
        }

        var rows = MoveLoader.LoadFromFile(path);
        var lorencia = rows.FirstOrDefault(r => r.Index == 2);
        Assert.NotNull(lorencia);
        Assert.Equal(17, lorencia!.Gate);
        Assert.Equal(10, lorencia.MinLevel);
    }

    static string ResolveMovePath()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_MOVE_PATH")?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        var repo = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", ".."));
        return Path.Combine(repo, "MuServer", "4.GameServer", "Data", "Move", "Move.txt");
    }
}

public sealed class MoveMapEquipRulesTests
{
    [Fact]
    public void BlocksAtlans_when_pet_is_dinorant()
    {
        var sid = Guid.NewGuid();
        var dinorant = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(dinorant, 13, 3, 0, 255, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 8, dinorant);

        Assert.True(MoveMapEquipRules.BlocksWarpToMap(MoveMapEquipRules.MapAtlans, sid, out var wearing));
        Assert.True(wearing);
    }

    [Fact]
    public void BlocksSkyMap_without_wings_or_mount()
    {
        var sid = Guid.NewGuid();
        Assert.True(MoveMapEquipRules.BlocksWarpToMap(MoveMapEquipRules.MapIcarus, sid, out _));
    }
}
