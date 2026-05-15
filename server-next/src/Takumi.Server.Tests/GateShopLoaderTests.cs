using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class GateShopLoaderTests
{
    static string Fixture(string name) =>
        Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "Fixtures", name));

    [Fact]
    public void GateLoader_parses_star_as_minus_one()
    {
        var path = ResolveGatePath();
        var gates = GateLoader.LoadFromFile(path);
        Assert.True(gates.Count >= 2, $"expected gates from {path}");
        var first = gates.First(g => g.GateIndex == 1);
        Assert.Equal(-1, first.MinReset);
        var second = gates.First(g => g.GateIndex == 2);
        Assert.Equal(0, second.TargetGate);
    }

    static string ResolveGatePath()
    {
        var sample = Fixture("Gate.sample.txt");
        if (File.Exists(sample) && GateLoader.LoadFromFile(sample).Count >= 2)
        {
            return sample;
        }

        var muserver = Path.GetFullPath(
            Path.Combine(
                AppContext.BaseDirectory,
                "..",
                "..",
                "..",
                "..",
                "..",
                "MuServer",
                "4.GameServer",
                "Sub 1",
                "Data",
                "Move",
                "Gate.txt"));
        if (File.Exists(muserver))
        {
            return muserver;
        }

        return sample;
    }

    [Fact]
    public void ShopManagerLoader_parses_wildcard_npc()
    {
        var path = Fixture("ShopManager.sample.txt");
        var shops = ShopManagerLoader.LoadFromFile(path);
        Assert.Equal(2, shops.Count);
        Assert.Null(shops[0].MapId);
        Assert.Equal((short)0, shops[1].MapId);
        Assert.Equal((short)120, shops[1].PosX);
    }

    [Fact]
    public void ShopItemLoader_parses_sixteen_columns()
    {
        var path = Fixture("Shop000.sample.txt");
        var items = ShopItemLoader.LoadFromFile(0, path);
        Assert.Single(items);
        Assert.Equal(0, items[0].ItemGroup);
        Assert.Equal(20, items[0].ItemIndex);
        Assert.Equal(255, items[0].Socket1);
    }
}
