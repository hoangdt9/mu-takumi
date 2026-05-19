using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterSpawnResolverLorenciaTests
{
    static readonly string? AttRoot = Environment.GetEnvironmentVariable("TAKUMI_ATT_DATA_ROOT");

    [Fact]
    public void Lorencia_field_box_spreads_instances_not_one_tile()
    {
        if (string.IsNullOrWhiteSpace(AttRoot) || !Directory.Exists(AttRoot))
        {
            return;
        }

        Environment.SetEnvironmentVariable("TAKUMI_ATT_DATA_ROOT", AttRoot);
        MapAttWalkability.PreloadMaps(new[] { (byte)0 });

        var entry = new MonsterSetBaseEntry
        {
            SpawnType = 1,
            MonsterClass = 3,
            Map = 0,
            X = 180,
            Y = 90,
            Tx = 226,
            Ty = 244,
        };

        var tiles = new HashSet<(byte X, byte Y)>();
        for (var i = 1; i <= 45; i++)
        {
            Assert.True(MonsterSpawnResolver.TryResolveFieldPosition(entry, i, out var x, out var y));
            Assert.False(MapAttWalkability.IsSafeZone(0, x, y));
            Assert.True(MapAttWalkability.CanWalk(0, x, y));
            tiles.Add((x, y));
        }

        Assert.True(tiles.Count >= 8, $"expected spread across box, got {tiles.Count} unique tiles");
    }
}
