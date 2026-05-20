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
    public void TryFindNextAddPointRequest_bulk_packet_decodes_stream_xor()
    {
        var packet = new byte[] { 0xC1, 0x07, 0xF3, 0x06, 0x00, 0x30, 0x75 };
        EncodeTakumiStreamXor(packet.AsSpan(), firstXorIndex: 3);
        var from = 0;

        Assert.True(CharacterStatPointHandler.TryFindNextAddPointRequest(packet, ref from, out var statType, out var count));
        Assert.Equal(0x00, statType);
        Assert.Equal(30000, count);
    }

    [Fact]
    public void TryFindNextAddPointRequest_ignores_f3_30_hotkey_save_even_when_stream_xor()
    {
        // Android move-map flow: SaveOptions() → C1 F3 30 (34 bytes) before C1 8E 02 move.
        var packet = new byte[]
        {
            0xC1, 0x22, 0xF3, 0x30, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x00,
            0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        };
        EncodeTakumiStreamXor(packet.AsSpan(), firstXorIndex: 3);
        var from = 0;

        Assert.False(CharacterStatPointHandler.TryFindNextAddPointRequest(packet, ref from, out _, out _));
        Assert.Equal(packet.Length, from);
    }

    [Fact]
    public void TryFindNextAddPointRequest_multiple_legacy_packets_aggregate_in_buffer()
    {
        var packet = new byte[]
        {
            0xC1, 0x05, 0xF3, 0x06, 0x01,
            0xC1, 0x05, 0xF3, 0x06, 0x01,
            0xC1, 0x05, 0xF3, 0x06, 0x01,
        };
        var from = 0;
        var total = 0;
        while (CharacterStatPointHandler.TryFindNextAddPointRequest(packet, ref from, out _, out var count))
        {
            total += count;
        }

        Assert.Equal(3, total);
        Assert.Equal(packet.Length, from);
    }

    [Fact]
    public void TryAddStatPoints_applies_bulk_and_caps_by_available_points()
    {
        var sheet = CharacterSheetStats.FromInts(100, 100, 100, 100, 0, 50000);
        const byte dkWire = 0x20;
        var applied = CharacterSheetCalculator.TryAddStatPoints(ref sheet, 0, 30000, dkWire, out _);

        Assert.Equal(30000, applied);
        Assert.Equal(30100, sheet.Strength);
        Assert.Equal(20000, sheet.LevelUpPoint);
    }

    [Fact]
    public void TryAddStatPoints_leadership_ignored_for_non_dark_lord()
    {
        var sheet = CharacterSheetStats.FromInts(26, 20, 20, 15, 25, 10);
        const byte mgWire = 120;
        var applied = CharacterSheetCalculator.TryAddStatPoints(ref sheet, 4, 3, mgWire, out _);

        Assert.Equal(0, applied);
        Assert.Equal(25, sheet.Leadership);
        Assert.Equal(10, sheet.LevelUpPoint);
    }

    [Fact]
    public void TryAddStatPoints_leadership_applies_for_dark_lord()
    {
        var sheet = CharacterSheetStats.FromInts(26, 20, 20, 15, 25, 10);
        const byte dlWire = 0x90;
        var applied = CharacterSheetCalculator.TryAddStatPoints(ref sheet, 4, 3, dlWire, out _);

        Assert.Equal(3, applied);
        Assert.Equal(28, sheet.Leadership);
        Assert.Equal(7, sheet.LevelUpPoint);
    }

    static void EncodeTakumiStreamXor(Span<byte> buffer, int firstXorIndex)
    {
        ReadOnlySpan<byte> filter =
        [
            0xAB, 0x11, 0xCD, 0xFE, 0x18, 0x23, 0xC5, 0xA3,
            0xCA, 0x33, 0xC1, 0xCC, 0x66, 0x67, 0x21, 0xF3,
            0x32, 0x12, 0x15, 0x35, 0x29, 0xFF, 0xFE, 0x1D,
            0x44, 0xEF, 0xCD, 0x41, 0x26, 0x3C, 0x4E, 0x4D,
        ];
        for (var i = firstXorIndex; i < buffer.Length; i++)
        {
            buffer[i] ^= (byte)(buffer[i - 1] ^ filter[i % 32]);
        }
    }
}
