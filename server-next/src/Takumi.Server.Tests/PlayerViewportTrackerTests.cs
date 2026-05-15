using Takumi.Server.Game.Networking;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class PlayerViewportTrackerTests
{
    [Fact]
    public void SyncPeers_tracks_enter_and_leave()
    {
        var tracker = new PlayerViewportTracker();
        tracker.ResetForMap(0, 100, 100);
        var (entered, left) = tracker.SyncPeers([1001, 1002]);
        Assert.Equal(2, entered.Count);
        Assert.Empty(left);

        var (entered2, left2) = tracker.SyncPeers([1002, 1003]);
        Assert.Single(entered2);
        Assert.Equal(1003, entered2[0]);
        Assert.Single(left2);
        Assert.Equal(1001, left2[0]);
    }
}
