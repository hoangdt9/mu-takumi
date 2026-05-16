using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class WarehouseWire602Tests
{
    [Fact]
    public void BuildMoney_has_expected_head()
    {
        var pkt = WarehouseWire602.BuildMoney(1000, 500);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(WarehouseWire602.MoneyHead, pkt[2]);
    }
}
