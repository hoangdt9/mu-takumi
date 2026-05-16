using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CharacterRosterJsonMigratorTests
{
    [Fact]
    public void TryLoadRowsFromJsonFile_loads_all_characters_in_file()
    {
        var dir = Path.Combine(Path.GetTempPath(), "roster-migrate-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(dir);
        var path = Path.Combine(dir, "test.json");
        File.WriteAllText(
            path,
            """
            {
              "characters": [
                { "name": "HeroA", "serverClass": 32, "level": 1, "mapId": 0, "posX": 10, "posY": 20, "angle": 1,
                "currentHp": 100, "maxHp": 100, "currentMp": 50, "maxMp": 50, "zen": 123 },
                { "name": "HeroB", "serverClass": 0, "level": 5, "mapId": 0, "posX": 30, "posY": 40, "angle": 2,
                "currentHp": 200, "maxHp": 220, "currentMp": 80, "maxMp": 90, "zen": 0 }
              ]
            }
            """);

        try
        {
            var rows = CharacterRosterJsonMigrator.TryLoadRowsFromJsonFile(path);
            Assert.Equal(2, rows.Count);
            Assert.Equal("HeroA", rows[0].Name);
            Assert.Equal("HeroB", rows[1].Name);
            Assert.Equal(123, rows[0].Zen);
        }
        finally
        {
            Directory.Delete(dir, recursive: true);
        }
    }

    [Fact]
    public void TryLoadRowsFromJsonFile_loads_experience_and_stats()
    {
        var dir = Path.Combine(Path.GetTempPath(), "roster-migrate-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(dir);
        var path = Path.Combine(dir, "acc.json");
        File.WriteAllText(
            path,
            """
            { "characters": [{ "name": "X", "serverClass": 0, "level": 8, "experience": 9999,
              "strength": 100, "dexterity": 50, "vitality": 40, "energy": 30, "leadership": 0,
              "levelUpPoint": 5, "mapId": 0, "posX": 1, "posY": 2, "angle": 0,
              "currentHp": 1, "maxHp": 1, "currentMp": 1, "maxMp": 1, "zen": 0 }] }
            """);

        try
        {
            var rows = CharacterRosterJsonMigrator.TryLoadRowsFromJsonFile(path);
            Assert.Single(rows);
            Assert.Equal(9999, rows[0].Experience);
            Assert.Equal(100, rows[0].Strength);
            Assert.Equal(5, rows[0].LevelUpPoint);
            Assert.Equal((byte)1, rows[0].PosX);
        }
        finally
        {
            Directory.Delete(dir, recursive: true);
        }
    }

    [Fact]
    public void Apply_spawn_defaults_when_xy_zero()
    {
        var dir = Path.Combine(Path.GetTempPath(), "roster-migrate-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(dir);
        var path = Path.Combine(dir, "acc.json");
        File.WriteAllText(
            path,
            """
            { "characters": [{ "name": "X", "serverClass": 0, "level": 1, "mapId": 0, "posX": 0, "posY": 0, "angle": 0,
              "currentHp": 1, "maxHp": 1, "currentMp": 1, "maxMp": 1, "zen": 0 }] }
            """);

        try
        {
            var rows = CharacterRosterJsonMigrator.TryLoadRowsFromJsonFile(path);
            Assert.Single(rows);
            Assert.Equal((byte)135, rows[0].PosX);
            Assert.Equal((byte)122, rows[0].PosY);
        }
        finally
        {
            Directory.Delete(dir, recursive: true);
        }
    }
}
