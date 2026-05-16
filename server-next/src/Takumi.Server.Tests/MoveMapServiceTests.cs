using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MoveMapServiceTests
{
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

        var ok = MoveMapService.TryResolve(2, playerLevel: 50, playerZen: 50000, previousMap: 0, out var dest, out var deny, out var zen);
        Assert.True(ok);
        Assert.Equal(MoveMapService.DenyReason.None, deny);
        Assert.Equal(2000, zen);
        Assert.NotEqual(2, dest.MapId);
    }

    [Fact]
    public void ToWireResult_maps_level_and_zen_denials()
    {
        Assert.Equal(MoveMapWire602.ResultNotEnoughLevel, MoveMapService.ToWireResult(MoveMapService.DenyReason.LevelTooLow));
        Assert.Equal(MoveMapWire602.ResultNotEnoughZen, MoveMapService.ToWireResult(MoveMapService.DenyReason.NotEnoughZen));
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
