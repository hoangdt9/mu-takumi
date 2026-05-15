using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapMonsterInstanceRegenTests
{
    [Fact]
    public void TryRegen_after_delay_restores_alive()
    {
        var m = Make(regenMs: 1);
        m.MarkDead();
        Assert.False(m.IsAlive);
        Thread.Sleep(5);
        Assert.True(m.TryRegen());
        Assert.True(m.IsAlive);
        Assert.Equal(50, m.CurrentLife);
    }

    [Fact]
    public void ApplyDamage_kills_at_zero_hp()
    {
        var m = Make(regenMs: 10_000);
        Assert.True(m.ApplyDamage(50));
        Assert.False(m.IsAlive);
        Assert.Equal(0, m.CurrentLife);
    }

    static MapMonsterInstance Make(int regenMs)
    {
        var m = new MapMonsterInstance
        {
            ObjectKey = 1,
            MonsterClass = 3,
            Map = 0,
            SpawnX = 10,
            SpawnY = 10,
            WanderLeash = 3,
            MoveRange = 3,
            MaxLife = 50,
            Level = 1,
            RegenDelayMs = regenMs,
        };
        m.InitializeAtSpawn(10, 10, 0);
        return m;
    }
}
