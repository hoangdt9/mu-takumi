using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterViewportTrackerTests
{
    [Fact]
    public void TakeNewInView_dedupes_by_object_key()
    {
        var tracker = new MonsterViewportTracker();
        tracker.ResetForMap(0, 100, 100);
        var m1 = Make(12001, 3, 105, 105);
        var m2 = Make(12002, 2, 106, 106);

        var first = tracker.TakeNewInView(new[] { m1, m2 });
        var second = tracker.TakeNewInView(new[] { m1, m2 });

        Assert.Equal(2, first.Count);
        Assert.Empty(second);
    }

    [Fact]
    public void ShouldRescan_after_move_threshold()
    {
        var tracker = new MonsterViewportTracker();
        tracker.ResetForMap(0, 100, 100);
        Assert.False(tracker.ShouldRescan(0, 102, 100, 4));
        Assert.True(tracker.ShouldRescan(0, 105, 100, 4));
    }

    static MapMonsterInstance Make(int key, int cls, byte x, byte y)
    {
        var m = new MapMonsterInstance
        {
            ObjectKey = key,
            MonsterClass = cls,
            Map = 0,
            X = x,
            Y = y,
            Dir = 3,
            MaxLife = 100,
            Level = 1,
            RegenDelayMs = 10_000,
        };
        m.InitializeLife();
        return m;
    }
}
