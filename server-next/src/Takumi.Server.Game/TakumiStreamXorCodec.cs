namespace Takumi.Server.Game;

/// <summary>
/// Inverse of Takumi stream <c>XorData</c> (same XOR chain as <c>SimpleModulusCrypt.h</c>).
/// </summary>
public static class TakumiStreamXorCodec
{
    public static void DecodeTakumiStreamXor(Span<byte> buffer, int firstXorIndex)
    {
        ReadOnlySpan<byte> filter =
        [
            0xAB, 0x11, 0xCD, 0xFE, 0x18, 0x23, 0xC5, 0xA3,
            0xCA, 0x33, 0xC1, 0xCC, 0x66, 0x67, 0x21, 0xF3,
            0x32, 0x12, 0x15, 0x35, 0x29, 0xFF, 0xFE, 0x1D,
            0x44, 0xEF, 0xCD, 0x41, 0x26, 0x3C, 0x4E, 0x4D,
        ];
        for (var i = buffer.Length - 1; i >= firstXorIndex; i--)
        {
            buffer[i] ^= (byte)(buffer[i - 1] ^ filter[i % 32]);
        }
    }
}
