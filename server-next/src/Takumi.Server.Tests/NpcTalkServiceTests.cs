using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class NpcTalkServiceTests
{
    [Theory]
    [InlineData(240, 2)]
    [InlineData(479, 33)]
    [InlineData(246, 255)]
    public void TryGetTalkResult_maps_known_classes(int monsterClass, byte expected)
    {
        if (expected == 255)
        {
            Assert.False(NpcTalkService.TryGetTalkResult(monsterClass, out _));
            return;
        }

        Assert.True(NpcTalkService.TryGetTalkResult(monsterClass, out var result));
        Assert.Equal(expected, result);
    }

    [Fact]
    public void NpcTalkWire_C1_length_matches_buffer()
    {
        var pkt = NpcTalkWire602.Build(NpcTalkService.TalkResultShop);
        Assert.Equal(9, pkt.Length);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(0x09, pkt[1]);
        Assert.Equal(NpcTalkWire602.Head, pkt[2]);
        Assert.Equal(NpcTalkService.TalkResultShop, pkt[3]);
    }

    [Fact]
    public void Warehouse_money_packet_layout()
    {
        var pkt = WarehouseWire602.BuildMoney(inventoryMoney: 1_000, warehouseMoney: 500);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(12, pkt[1]);
        Assert.Equal(0x81, pkt[2]);
        Assert.Equal(500u, BitConverter.ToUInt32(pkt, 4));
        Assert.Equal(1000u, BitConverter.ToUInt32(pkt, 8));
    }
}
