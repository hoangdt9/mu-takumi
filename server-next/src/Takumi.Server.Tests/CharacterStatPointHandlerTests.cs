using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CharacterStatPointHandlerTests
{
    [Fact]
    public void TryFindNextAddPointRequest_legacy_packet_count_is_one()
    {
        var packet = new byte[] { 0xC1, 0x05, 0xF3, 0x06, 0x02 };
        var from = 0;

        Assert.True(CharacterStatPointHandler.TryFindNextAddPointRequest(packet, ref from, out var statType, out var count));
        Assert.Equal(0x02, statType);
        Assert.Equal(1, count);
        Assert.Equal(packet.Length, from);
    }

    [Fact]
    public void TryFindNextAddPointRequest_bulk_packet_reads_count()
    {
        var packet = new byte[] { 0xC1, 0x07, 0xF3, 0x06, 0x00, 0x30, 0x75 }; // 30000 LE
        var from = 0;

        Assert.True(CharacterStatPointHandler.TryFindNextAddPointRequest(packet, ref from, out var statType, out var count));
        Assert.Equal(0x00, statType);
        Assert.Equal(30000, count);
    }

    [Fact]
    public void TryAddStatPoints_applies_bulk_and_caps_by_available_points()
    {
        var sheet = CharacterSheetStats.FromInts(100, 100, 100, 100, 0, 50000);
        var applied = CharacterSheetCalculator.TryAddStatPoints(ref sheet, 0, 30000, out _);

        Assert.Equal(30000, applied);
        Assert.Equal(30100, sheet.Strength);
        Assert.Equal(20000, sheet.LevelUpPoint);
    }
}
