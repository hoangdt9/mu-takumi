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
    }

    static MapMonsterInstance Make(int regenMs) =>
        new()
        {
            ObjectKey = 1,
            MonsterClass = 3,
            Map = 0,
            X = 10,
            Y = 10,
            Dir = 0,
            Life = 50,
            Level = 1,
            RegenDelayMs = regenMs,
        };
}
