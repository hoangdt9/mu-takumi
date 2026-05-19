using Takumi.Server.Game;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class GamePacketFindersLogoutTests
{
    [Fact]
    public void TryFindGameLogoutRequest_plain_c1_f1_02()
    {
        var packet = new byte[] { 0xC1, 0x05, 0xF1, 0x02, 0x01 };

        Assert.True(GamePacketFinders.TryFindGameLogoutRequest(packet, out var off, out var flag));
        Assert.Equal(0, off);
        Assert.Equal(1, flag);
    }

    [Fact]
    public void TryFindGameLogoutRequest_c1_stream_xor_after_sendrequestlogout()
    {
        // wsclientinline.h SendRequestLogOut: C1 len F1 02 flag — StreamPacketEngine XORs from byte index 3.
        var packet = new byte[] { 0xC1, 0x05, 0xF1, 0x02, 0x01 };
        EncodeTakumiStreamXor(packet.AsSpan(), firstXorIndex: 3);

        Assert.False(packet[3] == 0x02);

        Assert.True(GamePacketFinders.TryFindGameLogoutRequest(packet, out var off, out var flag));
        Assert.Equal(0, off);
        Assert.Equal(1, flag);
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
