using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapMonsterChaseTests
{
    [Fact]
    public void TryChaseStep_moves_closer_to_target()
    {
        var m = new MapMonsterInstance
        {
            ObjectKey = 12001,
            MonsterClass = 3,
            Map = 0,
            SpawnX = 50,
            SpawnY = 50,
            WanderLeash = 10,
            MoveRange = 10,
            MaxLife = 50,
            Level = 5,
            RegenDelayMs = 5000,
            IsNpc = false,
        };
        m.InitializeAtSpawn(50, 50, 0);

        Assert.True(m.TryChaseStep(52, 50, mapId: 99, out _, out _, out _));
        Assert.True(m.ManhattanTo(52, 50) < 3);
    }
}
