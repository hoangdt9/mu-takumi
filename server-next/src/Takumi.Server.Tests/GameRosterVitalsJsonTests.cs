using System.Text;
using System.Text.Json;
using Takumi.Server.Game;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class GameRosterVitalsJsonTests
{
    [Fact]
    public void Json_round_trip_preserves_vitals()
    {
        var root = new
        {
            characters = new[]
            {
                new
                {
                    name = "Hero",
                    serverClass = (byte)0x20,
                    level = (ushort)10,
                    mapId = (byte)0,
                    posX = (byte)135,
                    posY = (byte)122,
                    angle = (byte)1,
                    currentHp = 321,
                    maxHp = 500,
                    currentMp = 80,
                    maxMp = 120,
                    zen = 999L,
                },
            },
        };

        var json = JsonSerializer.Serialize(root, GameRosterDisk.JsonOptions);
        var account = "vit" + Guid.NewGuid().ToString("N");
        var dir = Path.Combine(Path.GetTempPath(), "takumi-roster-vitals-" + Guid.NewGuid().ToString("N"));
        var path = GameRosterDisk.GetRosterFilePath(account);
        try
        {
            Directory.CreateDirectory(dir);
            var originalDir = Environment.GetEnvironmentVariable("TAKUMI_ROSTER_DIR");
            Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DIR", dir);
            path = GameRosterDisk.GetRosterFilePath(account);
            File.WriteAllText(path, json);

            try
            {
                var loaded = GameRosterDisk.LoadEntries(account);
                Assert.Single(loaded);
                Assert.Equal(321, loaded[0].CurrentHp);
                Assert.Equal(500, loaded[0].MaxHp);
                Assert.Equal(80, loaded[0].CurrentMp);
                Assert.Equal(120, loaded[0].MaxMp);
                Assert.Equal(999, loaded[0].Zen);
            }
            finally
            {
                if (originalDir is null)
                {
                    Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DIR", null);
                }
                else
                {
                    Environment.SetEnvironmentVariable("TAKUMI_ROSTER_DIR", originalDir);
                }
            }
        }
        finally
        {
            if (File.Exists(path))
            {
                File.Delete(path);
            }

            if (Directory.Exists(dir))
            {
                Directory.Delete(dir, recursive: true);
            }
        }
    }
}
