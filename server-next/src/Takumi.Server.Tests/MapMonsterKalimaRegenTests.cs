using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapMonsterKalimaRegenTests
{
    [Fact]
    public void TryRegen_on_kalima_map_restores_at_death_tile()
    {
        var mob = new MapMonsterInstance
        {
            ObjectKey = 9001,
            MonsterClass = 10,
            Map = 24,
            SpawnX = 10,
            SpawnY = 10,
            WanderLeash = 3,
            MoveRange = 3,
            MaxLife = 100,
            Level = 1,
            RegenDelayMs = 0,
        };
        mob.InitializeAtSpawn(10, 10, 0);

        Assert.True(mob.TryChaseStep(12, 11, 24, out var deathX, out var deathY, out _));
        mob.ApplyDamage(200);
        Assert.False(mob.IsAlive);

        Assert.True(mob.TryRegen());
        Assert.True(mob.IsAlive);
        Assert.Equal(deathX, mob.X);
        Assert.Equal(deathY, mob.Y);
        Assert.NotEqual((byte)10, mob.X);
    }

    [Fact]
    public void TryRegen_on_normal_map_restores_at_spawn()
    {
        var mob = new MapMonsterInstance
        {
            ObjectKey = 9002,
            MonsterClass = 10,
            Map = 0,
            SpawnX = 20,
            SpawnY = 20,
            WanderLeash = 3,
            MoveRange = 3,
            MaxLife = 100,
            Level = 1,
            RegenDelayMs = 0,
        };
        mob.InitializeAtSpawn(20, 20, 0);
        mob.ApplyDamage(200);
        Assert.True(mob.TryRegen());
        Assert.Equal((byte)20, mob.X);
        Assert.Equal((byte)20, mob.Y);
    }
}
