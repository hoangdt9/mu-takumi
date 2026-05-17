using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapAttWalkabilityLorenciaTests
{
    static readonly string? AttRoot = Environment.GetEnvironmentVariable("TAKUMI_ATT_DATA_ROOT");

    [Fact]
    public void Lorencia_133_168_walkability_when_att_mounted()
    {
        if (string.IsNullOrWhiteSpace(AttRoot) || !Directory.Exists(AttRoot))
        {
            return; // skip when CI has no MU data
        }

        Environment.SetEnvironmentVariable("TAKUMI_ATT_DATA_ROOT", AttRoot);
        var canWalk = MapAttWalkability.CanWalk(0, 133, 168);
        var healed = MapAttWalkability.TryFindNearestWalkable(0, 133, 168, out var wx, out var wy);
        Assert.True(healed, $"expected walkable neighbor for (133,168); canWalkCenter={canWalk} healed=({wx},{wy})");
    }
}
