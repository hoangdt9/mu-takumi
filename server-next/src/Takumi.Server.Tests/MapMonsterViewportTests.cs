using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapMonsterViewportTests
{
    [Fact]
    public void GetViewportEntities_prioritizes_npcs_by_distance()
    {
        Environment.SetEnvironmentVariable("TAKUMI_MONSTER_SET_BASE_PATH", string.Empty);
        Environment.SetEnvironmentVariable("TAKUMI_MONSTER_INFO_PATH", string.Empty);

        var npcFar = Create(1, isNpc: true, x: 50, y: 50);
        var npcNear = Create(2, isNpc: true, x: 10, y: 10);
        var mobNear = Create(3, isNpc: false, x: 11, y: 10);
        var all = new List<MapMonsterInstance> { npcFar, mobNear, npcNear };

        var view = SelectViewport(all, px: 10, py: 10, viewRange: 20, maxNpcs: 4, maxMobs: 4);

        Assert.Equal(2, view.Count);
        Assert.True(view[0].IsNpc);
        Assert.Equal(2, view[0].ObjectKey);
        Assert.False(view[1].IsNpc);
    }

    static IReadOnlyList<MapMonsterInstance> SelectViewport(
        List<MapMonsterInstance> all,
        byte px,
        byte py,
        int viewRange,
        int maxNpcs,
        int maxMobs)
    {
        var npcs = new List<(int Dist, MapMonsterInstance Mob)>();
        var mobs = new List<(int Dist, MapMonsterInstance Mob)>();
        foreach (var m in all)
        {
            var dist = Math.Abs(m.X - px) + Math.Abs(m.Y - py);
            if (dist > viewRange)
            {
                continue;
            }

            if (m.IsNpc)
            {
                npcs.Add((dist, m));
            }
            else
            {
                mobs.Add((dist, m));
            }
        }

        npcs.Sort(static (a, b) => a.Dist.CompareTo(b.Dist));
        mobs.Sort(static (a, b) => a.Dist.CompareTo(b.Dist));

        var result = new List<MapMonsterInstance>();
        foreach (var (_, m) in npcs)
        {
            if (result.Count >= maxNpcs)
            {
                break;
            }

            result.Add(m);
        }

        foreach (var (_, m) in mobs)
        {
            if (result.Count >= maxNpcs + maxMobs)
            {
                break;
            }

            result.Add(m);
        }

        return result;
    }

    static MapMonsterInstance Create(int key, bool isNpc, byte x, byte y)
    {
        var inst = new MapMonsterInstance
        {
            ObjectKey = key,
            MonsterClass = isNpc ? 226 : 3,
            Map = 0,
            SpawnX = x,
            SpawnY = y,
            WanderLeash = 3,
            MoveRange = 3,
            MaxLife = 100,
            Level = isNpc ? 0 : 5,
            RegenDelayMs = 10_000,
            IsNpc = isNpc,
        };
        inst.InitializeAtSpawn(x, y, 0);
        return inst;
    }
}
