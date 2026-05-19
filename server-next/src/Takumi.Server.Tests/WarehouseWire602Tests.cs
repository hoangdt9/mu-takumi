using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class WarehouseWire602Tests
{
    [Fact]
    public void Storage_gold_request_parses_flag_and_amount()
    {
        var pkt = new byte[] { 0xC1, 0x08, 0x81, 0x01, 0x10, 0x27, 0x00, 0x00 };
        Assert.True(WarehouseWire602.TryFindStorageGoldRequest(pkt, out _, out var flag, out var gold));
        Assert.Equal((byte)1, flag);
        Assert.Equal(10_000u, gold);
    }

    [Fact]
    public void Storage_exit_request_parses_c1_82()
    {
        var pkt = new byte[] { 0xC1, 0x03, 0x82 };
        Assert.True(WarehouseWire602.TryFindStorageExitRequest(pkt, out var off));
        Assert.Equal(0, off);
    }
}
