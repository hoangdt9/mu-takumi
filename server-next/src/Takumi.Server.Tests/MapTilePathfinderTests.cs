using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapTilePathfinderTests
{
    [Fact]
    public void TryFindNextStep_moves_toward_target_when_all_walkable()
    {
        var d0 = Math.Abs(10 - 14) + Math.Abs(10 - 10);
        Assert.True(MapTilePathfinder.TryFindNextStep(99, 10, 10, 14, 10, 16, out var nx, out var ny));
        var d1 = Math.Abs(nx - 14) + Math.Abs(ny - 10);
        Assert.True(d1 < d0);
    }
}
