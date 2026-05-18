using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapMonsterSpawnCoverageTests
{
    [Fact]
    public void Real_monster_set_base_has_field_mobs_on_move_town_maps()
    {
        var path = ResolveMonsterSetBase();
        if (path is null || !File.Exists(path))
        {
            return;
        }

        var entries = MonsterSetBaseLoader.LoadFromFile(path);
        var monsterTxt = Path.Combine(Path.GetDirectoryName(path)!, "Monster.txt");
        var stats = File.Exists(monsterTxt)
            ? MonsterStatCatalog.LoadFromFile(monsterTxt)
            : new MonsterStatCatalog();
        var instances = BuildInstances(entries, stats);
        var byMap = instances
            .GroupBy(i => i.Map)
            .ToDictionary(g => g.Key, g => new MapMonsterSpawnCoverage.MapSpawnSummary(
                g.Count(),
                g.Count(m => m.IsNpc),
                g.Count(m => !m.IsNpc)));

        foreach (var mapId in new byte[] { 0, 2, 3 })
        {
            Assert.True(byMap.TryGetValue(mapId, out var summary), $"map {mapId} missing in {path}");
            Assert.True(summary.Total > 0, $"map {mapId} has no spawns in {path}");
        }

        foreach (var mapId in new byte[] { 0, 2, 3 })
        {
            var summary = byMap[mapId];
            Assert.True(
                summary.FieldCount > 0,
                $"map {mapId} has no field monsters in {path} (total={summary.Total} npc={summary.NpcCount})");
        }
    }

    static string? ResolveMonsterSetBase()
    {
        var candidates = new[]
        {
            Path.GetFullPath(
                Path.Combine(
                    AppContext.BaseDirectory,
                    "..",
                    "..",
                    "..",
                    "..",
                    "..",
                    "..",
                    "MuServer",
                    "4.GameServer",
                    "Data",
                    "Monster",
                    "MonsterSetBase.txt")),
            Path.GetFullPath(
                Path.Combine(
                    AppContext.BaseDirectory,
                    "Fixtures",
                    "MonsterSetBase.m9test.txt")),
        };

        foreach (var c in candidates)
        {
            if (File.Exists(c))
            {
                return c;
            }
        }

        return null;
    }

    static List<MapMonsterInstance> BuildInstances(
        IReadOnlyList<MonsterSetBaseEntry> setBase,
        MonsterStatCatalog stats)
    {
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
