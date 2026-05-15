using System.Text;
using Takumi.Server.Game.World;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class InventorySlotMirrorWriterTests
{
    [Fact]
    public void ScheduleUpsert_with_no_repo_does_not_throw()
    {
        InventorySlotMirrorWriter.ScheduleUpsertSlot("test", "hero", 12, new byte[ItemWire602.WireBytes]);
        Assert.Equal(0, InventorySlotMirrorWriter.PendingOpsForTests);
    }

    [Fact]
    public void PersistSlotToMirror_empty_slot_schedules_delete_when_no_repo()
    {
        PlayerShopSession.PersistSlotToMirror("test", Encoding.ASCII.GetBytes("hero000000"), 12, new byte[ItemWire602.WireBytes]);
        Assert.Equal(0, InventorySlotMirrorWriter.PendingOpsForTests);
    }

    [Fact]
    public void BuildSlotSnapshot_returns_non_empty_after_set()
    {
        var sid = Guid.NewGuid();
        var blob = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(blob, 14, 3, 5, 40, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 12, blob);
        var snap = PlayerShopSession.BuildSlotSnapshot(sid);
        Assert.Single(snap);
        Assert.Equal(12, snap[0].Slot);
        PlayerShopSession.RemoveSession(sid);
    }
}
