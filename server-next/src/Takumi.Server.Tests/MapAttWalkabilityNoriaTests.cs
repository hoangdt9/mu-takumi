using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapAttWalkabilityNoriaTests
{
    static readonly string? AttRoot = ResolveAttRoot();

    static string? ResolveAttRoot()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_ATT_DATA_ROOT");
        if (!string.IsNullOrWhiteSpace(env) && Directory.Exists(env))
        {
            return env;
        }

        var candidates = new[]
        {
            Path.GetFullPath(
                Path.Combine(
                    AppContext.BaseDirectory,
                    "..",
                    "..",
                    "..",
                    "..",
                    "..",
                    "ClientBuild_192.168.99.200",
                    "Data")),
            Path.GetFullPath(
                Path.Combine(
                    AppContext.BaseDirectory,
                    "..",
                    "..",
                    "..",
                    "..",
                    "..",
                    "..",
                    "ClientBuild_192.168.99.200",
                    "Data")),
        };

        foreach (var c in candidates)
        {
            if (Directory.Exists(Path.Combine(c, "World4")))
            {
                return c;
            }
        }

        return null;
    }

    [Fact]
    public void Noria_gate_172_113_is_safe_zone_when_world4_att_present()
    {
        if (string.IsNullOrWhiteSpace(AttRoot))
        {
            return;
        }

        Environment.SetEnvironmentVariable("TAKUMI_ATT_DATA_ROOT", AttRoot);
        Assert.True(MapAttWalkability.IsAttLoaded(3), $"ATT not loaded from {AttRoot}");
        Assert.True(
            MapAttWalkability.IsSafeZone(3, 172, 113),
            "Noria town gate (172,113) should be safe zone when EncTerrain4.att is mounted");
    }

    [Fact]
    public void Field_spawn_resolver_avoids_noria_town_tile()
    {
        if (string.IsNullOrWhiteSpace(AttRoot))
        {
            return;
        }

        Environment.SetEnvironmentVariable("TAKUMI_ATT_DATA_ROOT", AttRoot);
        var entry = new MonsterSetBaseEntry
        {
            SpawnType = 1,
            MonsterClass = 3,
            Map = 3,
            X = 172,
            Y = 113,
            Tx = 172,
            Ty = 113,
        };

        Assert.True(MonsterSpawnResolver.TryResolveFieldPosition(entry, out var x, out var y));
        Assert.False(MapAttWalkability.IsSafeZone(3, x, y));
    }
}
