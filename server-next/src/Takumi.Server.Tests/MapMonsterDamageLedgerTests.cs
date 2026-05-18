using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapMonsterDamageLedgerTests
{
    [Fact]
    public void CopyDamageContributors_lists_all_positive_hits()
    {
        var mob = new MapMonsterInstance
        {
            ObjectKey = 1,
            MonsterClass = 3,
            Map = 0,
            SpawnX = 1,
            SpawnY = 1,
            WanderLeash = 3,
            MoveRange = 3,
            MaxLife = 100,
            Level = 1,
            RegenDelayMs = 1000,
        };
        mob.InitializeAtSpawn(1, 1, 0);
        mob.RecordHit(100, 40);
        mob.RecordHit(200, 60);

        var list = new List<(int, int)>();
        mob.CopyDamageContributors(list);
        Assert.Equal(2, list.Count);
        Assert.Equal(100, mob.TotalRecordedDamage());
    }
}
