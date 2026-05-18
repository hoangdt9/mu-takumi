using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapRespawnCatalogTests
{
    [Fact]
    public void Lorencia_uses_join_default()
    {
        var s = MapRespawnCatalog.GetTownRespawn(0);
        Assert.Equal(JoinMapSpawnWire.LorenciaDefault, s);
    }

    [Fact]
    public void Noria_uses_noria_town_not_lorencia_tile()
    {
        var s = MapRespawnCatalog.GetTownRespawn(3);
        Assert.Equal((byte)3, s.Map);
        Assert.NotEqual(JoinMapSpawnWire.LorenciaDefault.PositionX, s.PositionX);
        Assert.NotEqual(JoinMapSpawnWire.LorenciaDefault.PositionY, s.PositionY);
        Assert.Equal(173, s.PositionX);
        Assert.Equal(125, s.PositionY);
    }

    [Fact]
    public void Devias_dungeon_losttower_atlans_have_distinct_tiles()
    {
        var d = MapRespawnCatalog.GetTownRespawn(2);
        var du = MapRespawnCatalog.GetTownRespawn(1);
        var lt = MapRespawnCatalog.GetTownRespawn(4);
        var at = MapRespawnCatalog.GetTownRespawn(7);
        Assert.Equal((byte)2, d.Map);
        Assert.Equal(183, d.PositionX);
        Assert.Equal((byte)1, du.Map);
        Assert.Equal((byte)4, lt.Map);
        Assert.Equal((byte)7, at.Map);
        var tk = MapRespawnCatalog.GetTownRespawn(8);
        var ic = MapRespawnCatalog.GetTownRespawn(10);
        Assert.Equal((byte)8, tk.Map);
        Assert.Equal(195, tk.PositionX);
        Assert.Equal((byte)10, ic.Map);
        Assert.Equal(15, ic.PositionX);
    }
}
