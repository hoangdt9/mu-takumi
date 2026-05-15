using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ItemValueWire602Tests
{
    [Fact]
    public void Item_value_C2_F3_E9_header()
    {
        var pkt = ItemValueWire602.Build(
        [
            new ItemValueWire602.ItemValueEntry(100, 0, 0, 0, 500, 0, 150),
        ]);
        Assert.Equal(0xC2, pkt[0]);
        Assert.Equal(0xF3, pkt[3]);
        Assert.Equal(0xE9, pkt[4]);
        Assert.Equal(1, BitConverter.ToInt32(pkt, 5));
    }
}
