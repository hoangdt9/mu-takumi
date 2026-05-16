namespace Takumi.Server.Protocol;

/// <summary>Minimal guild wire acks so clients do not hang on unsupported guild UI.</summary>
public static class GuildWire602
{
    public static bool TryFindGuildPacket(ReadOnlySpan<byte> packet, out byte headCode)
    {
        headCode = 0;
        for (var i = 0; i <= packet.Length - 3; i++)
        {
            if (packet[i] != 0xC1)
            {
                continue;
            }

            var code = packet[i + 2];
            if (code is >= 0x50 and <= 0x56 or 0x5D or >= 0x60 and <= 0x66 or 0xE1 or 0xE5 or 0xE6 or 0xEB or 0x67)
            {
                headCode = code;
                return true;
            }
        }

        return false;
    }

    /// <summary>Generic failure/empty ack (result byte 0).</summary>
    public static byte[] BuildAck(byte headCode, byte result = 0)
    {
        var buf = new byte[5];
        buf[0] = 0xC1;
        buf[1] = 5;
        buf[2] = headCode;
        buf[3] = result;
        return buf;
    }
}
