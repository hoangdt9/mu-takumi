using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary><c>C1 F3 30</c> option / skill hotkey wire (Season 6 Takumi client).</summary>
public static class OptionDataWire602
{
    public const byte Head = 0xF3;
    public const byte SubCode = 0x30;
    public const int ConfigurationLength = CharacterKeyConfiguration.Length;

    /// <summary>Build server → client <c>PRECEIVE_OPTION</c> frame.</summary>
    public static byte[] BuildApply(ReadOnlySpan<byte> configuration)
    {
        var config = CharacterKeyConfiguration.Normalize(configuration);
        var packet = new byte[4 + ConfigurationLength];
        packet[0] = 0xC1;
        packet[1] = (byte)packet.Length;
        packet[2] = Head;
        packet[3] = SubCode;
        config.CopyTo(packet.AsSpan(4));
        return packet;
    }

    public static bool TryFindSaveRequest(
        ReadOnlySpan<byte> packet,
        out ReadOnlySpan<byte> configuration)
    {
        configuration = ReadOnlySpan<byte>.Empty;
        for (var i = 0; i + 4 + ConfigurationLength <= packet.Length; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 2] != Head)
            {
                continue;
            }

            var frameLen = packet[i + 1];
            if (frameLen < 4 + ConfigurationLength || i + frameLen > packet.Length)
            {
                continue;
            }

            if (packet[i + 3] == SubCode)
            {
                configuration = packet.Slice(i + 4, ConfigurationLength);
                return true;
            }

            // Client StreamPacketEngine XORs from index 3 before send — logical F3 30 often appears as F3 0x3D at +3.
            if (frameLen != 4 + ConfigurationLength)
            {
                continue;
            }

            Span<byte> scratch = stackalloc byte[4 + ConfigurationLength];
            packet.Slice(i, frameLen).CopyTo(scratch);
            TakumiStreamXorCodec.DecodeTakumiStreamXor(scratch, firstXorIndex: 3);
            if (scratch[2] != Head || scratch[3] != SubCode)
            {
                continue;
            }

            // Cannot return a span into `scratch` from this method (ref safety); copy decoded blob.
            var decoded = new byte[ConfigurationLength];
            scratch.Slice(4, ConfigurationLength).CopyTo(decoded);
            configuration = decoded;
            return true;
        }

        return false;
    }
}
