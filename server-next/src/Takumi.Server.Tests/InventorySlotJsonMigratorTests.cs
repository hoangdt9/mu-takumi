using Takumi.Server.Persistence;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class InventorySlotJsonMigratorTests
{
    [Fact]
    public void TryLoadSlotsFromJsonFile_parses_item_hex()
    {
        var dir = Path.Combine(Path.GetTempPath(), "inv-migrate-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(dir);
        var path = Path.Combine(dir, "test1.json");
        var hex = new string('A', ItemWire602.WireBytes * 2);
        File.WriteAllText(
            path,
            $$"""
            {
              "characters": [
                {
                  "name": "dk001",
                  "slots": [ { "slot": 12, "itemHex": "{{hex}}" } ]
                }
              ]
            }
            """);

        try
        {
            var map = InventorySlotJsonMigrator.TryLoadSlotsFromJsonFile(path);
            Assert.Single(map);
            Assert.True(map.ContainsKey("dk001"));
            Assert.Single(map["dk001"]);
            Assert.Equal((byte)12, map["dk001"][0].Slot);
            Assert.Equal(ItemWire602.WireBytes, map["dk001"][0].Item12.Length);
            Assert.All(map["dk001"][0].Item12, b => Assert.Equal(0xAA, b));
        }
        finally
        {
            Directory.Delete(dir, recursive: true);
        }
    }
}
