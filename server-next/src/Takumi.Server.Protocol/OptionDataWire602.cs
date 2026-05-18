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
            if (packet[i] != 0xC1 || packet[i + 2] != Head || packet[i + 3] != SubCode)
            {
                continue;
            }

            var frameLen = packet[i + 1];
            if (frameLen < 4 + ConfigurationLength || i + frameLen > packet.Length)
            {
                continue;
            }

            configuration = packet.Slice(i + 4, ConfigurationLength);
            return true;
        }

        return false;
    }
}
