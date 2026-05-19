using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class OptionDataWire602Tests
{
    [Fact]
    public void BuildApply_has_expected_header_and_length()
    {
        var config = CharacterKeyConfiguration.CreateDefault();
        config[0] = 0x12;
        config[1] = 0x34;

        var pkt = OptionDataWire602.BuildApply(config);

        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(34, pkt[1]);
        Assert.Equal(0xF3, pkt[2]);
        Assert.Equal(0x30, pkt[3]);
        Assert.Equal(0x12, pkt[4]);
        Assert.Equal(0x34, pkt[5]);
    }

    [Fact]
    public void TryFindSaveRequest_round_trips_client_frame()
    {
        var config = CharacterKeyConfiguration.CreateDefault();
        var frame = OptionDataWire602.BuildApply(config);

        Assert.True(OptionDataWire602.TryFindSaveRequest(frame, out var parsed));
        Assert.Equal(config.Length, parsed.Length);
        Assert.True(parsed.SequenceEqual(config));
    }

    [Fact]
    public void TryFindSaveRequest_decodes_stream_xor_like_client_send()
    {
        var config = CharacterKeyConfiguration.CreateDefault();
        config[0] = 0x12;
        config[1] = 0x34;
        var frame = OptionDataWire602.BuildApply(config);
        EncodeTakumiStreamXor(frame.AsSpan(), firstXorIndex: 3);

        Assert.NotEqual(0x30, frame[3]);

        Assert.True(OptionDataWire602.TryFindSaveRequest(frame, out var parsed));
        Assert.True(parsed.SequenceEqual(config));
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
