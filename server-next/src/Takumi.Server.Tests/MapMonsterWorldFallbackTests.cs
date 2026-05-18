using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapMonsterWorldSpawnFilterTests
{
    [Fact]
    public void Rebuild_skips_invasion_section_and_keeps_npc_section0()
    {
        var path = Path.GetFullPath(
            Path.Combine(AppContext.BaseDirectory, "Fixtures", "MonsterSetBase.m9test.txt"));
        var entries = MonsterSetBaseLoader.LoadFromFile(path);
        Assert.Contains(entries, e => e.SpawnType == 3);
        Assert.Contains(entries, e => e is { SpawnType: 0, MonsterClass: 479, X: 130, Y: 133 });

        var instances = BuildInstances(entries);
        Assert.DoesNotContain(instances, i => i.MonsterClass == 3);
        Assert.Contains(instances, i => i is { MonsterClass: 479, X: 130, Y: 133, IsNpc: true });
    }

    [Fact]
    public void ShouldIncludeSpawnEntry_includes_invasion_when_env_set()
    {
        var invasion = new MonsterSetBaseEntry { SpawnType = 3, MonsterClass = 99, Map = 0 };
        try
        {
            Environment.SetEnvironmentVariable("TAKUMI_MONSTER_INCLUDE_INVASION_SPAWN", "1");
            Assert.True(MapMonsterWorld.ShouldIncludeSpawnEntry(invasion));
        }
        finally
        {
            Environment.SetEnvironmentVariable("TAKUMI_MONSTER_INCLUDE_INVASION_SPAWN", null);
        }
    }

    static List<MapMonsterInstance> BuildInstances(IReadOnlyList<MonsterSetBaseEntry> setBase)
    {
        var stats = new MonsterStatCatalog();
        var list = new List<MapMonsterInstance>();
        var key = 12_000;
        foreach (var e in setBase)
        {
            if (!MapMonsterWorld.ShouldIncludeSpawnEntry(e))
            {
                continue;
            }

            if (!MonsterSpawnResolver.TryResolvePosition(e, out var x, out var y))
            {
                continue;
            }

            var stat = stats.GetOrDefault(e.MonsterClass);
            var isNpc = MonsterNpcClassifier.IsNpc(e.MonsterClass) || stat.Level == 0;
            var inst = new MapMonsterInstance
            {
                ObjectKey = key++,
                MonsterClass = e.MonsterClass,
                Map = e.Map,
                SpawnX = x,
                SpawnY = y,
                WanderLeash = 3,
                MoveRange = 3,
                MaxLife = isNpc ? 1 : stat.Life,
                Level = stat.Level,
                RegenDelayMs = 60_000,
                IsNpc = isNpc,
            };
            inst.InitializeAtSpawn(x, y, e.Dir);
            list.Add(inst);
        }

        return list;
    }
}
