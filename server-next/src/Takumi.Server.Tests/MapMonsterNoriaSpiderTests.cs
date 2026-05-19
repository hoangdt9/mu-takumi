using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapMonsterNoriaSpiderTests
{
    [Fact]
    public void ShouldIncludeSpawnEntry_excludes_spider_field_spawns_on_noria()
    {
        var spiderNoria = new MonsterSetBaseEntry
        {
            SpawnType = 1,
            MonsterClass = 3,
            Map = 3,
            Dis = 30,
            X = 139,
            Y = 63,
            Tx = 247,
            Ty = 164,
            Dir = 255,
        };

        Assert.False(MapMonsterWorld.ShouldIncludeSpawnEntry(spiderNoria));
    }

    [Fact]
    public void ShouldIncludeSpawnEntry_keeps_lorencia_spider_field_spawns()
    {
        var spiderLorencia = new MonsterSetBaseEntry
        {
            SpawnType = 1,
            MonsterClass = 3,
            Map = 0,
            Dis = 30,
            X = 180,
            Y = 90,
            Tx = 226,
            Ty = 244,
            Dir = 255,
        };

        Assert.True(MapMonsterWorld.ShouldIncludeSpawnEntry(spiderLorencia));
    }
}
