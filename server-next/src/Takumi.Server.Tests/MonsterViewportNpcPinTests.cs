using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterViewportNpcPinTests
{
    [Fact]
    public void SyncView_does_not_destroy_npc_keys_by_default()
    {
        MapMonsterWorld.EnsureInitialized();
        var npc = MapMonsterWorld.GetMonstersOnMap(0).FirstOrDefault(m => m.IsNpc);
        if (npc is null)
        {
            return;
        }

        var tracker = new MonsterViewportTracker();
        tracker.ResetForMap(0, npc.X, npc.Y);
        _ = tracker.TakeNewInView([npc]);

        var far = MapMonsterWorld.GetViewportEntities(0, 0, 0, viewRange: 1, maxNpcs: 80, maxMonsters: 80);
        var (_, left) = tracker.SyncView(far);

        Assert.DoesNotContain(npc.ObjectKey, left);
    }
}
