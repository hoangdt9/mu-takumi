using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapMonsterInstanceWanderTests
{
    [Fact]
    public void TryRollWander_stays_within_leash()
    {
        var m = new MapMonsterInstance
        {
            ObjectKey = 12001,
            MonsterClass = 3,
            Map = 0,
            SpawnX = 20,
            SpawnY = 20,
            WanderLeash = 2,
            MoveRange = 2,
            MaxLife = 50,
            Level = 5,
            RegenDelayMs = 5000,
            IsNpc = false,
        };
        m.InitializeAtSpawn(20, 20, 0);

        var rng = new Random(42);
        for (var i = 0; i < 20; i++)
        {
            Assert.True(m.TryRollWander(rng, mapId: 0, out var nx, out var ny, out _));
            Assert.True(Math.Abs(nx - 20) + Math.Abs(ny - 20) <= 2);
        }
    }

    [Fact]
    public void Npc_does_not_wander()
    {
        var m = new MapMonsterInstance
        {
            ObjectKey = 12002,
            MonsterClass = 226,
            Map = 0,
            SpawnX = 10,
            SpawnY = 10,
            WanderLeash = 3,
            MoveRange = 3,
            MaxLife = 1,
            Level = 0,
            RegenDelayMs = int.MaxValue / 2,
            IsNpc = true,
        };
        m.InitializeAtSpawn(10, 10, 0);

        Assert.False(m.TryRollWander(Random.Shared, mapId: 0, out _, out _, out _));
    }
}
