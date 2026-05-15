using Takumi.Server.Persistence;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class InventoryStagingImporterTests
{
    [Fact]
    public void EncodeItem_builds_12_byte_wire_from_flat_index()
    {
        var blob = InventoryStagingImporter.EncodeItem(
            flatIndex: 14 * 512 + 3,
            level: 5,
            durability: 40,
            skill: false,
            luck: true,
            option: 0,
            excellent: 0);
        Assert.Equal(14 * 512 + 3, ItemWire602.DecodeItemIndex(blob));
        Assert.Equal(5, ItemWire602.DecodeLevel(blob));
    }
}
