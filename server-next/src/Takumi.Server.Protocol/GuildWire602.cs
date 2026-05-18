namespace Takumi.Server.Protocol;

/// <summary>Minimal guild wire helpers until full guild domain exists.</summary>
public static class GuildWire602
{
    private const byte MinC1GuildFrame = 3;

    /// <summary>True when <paramref name="packet"/> is a single leading C1 guild frame (not a scan inside C3 payloads).</summary>
    public static bool TryParseLeadingC1Frame(ReadOnlySpan<byte> packet, out byte headCode, out int frameLength)
    {
        headCode = 0;
        frameLength = 0;
        if (packet.Length < MinC1GuildFrame || packet[0] != 0xC1)
        {
            return false;
        }

        frameLength = packet[1];
        if (frameLength < MinC1GuildFrame || frameLength > packet.Length)
        {
            return false;
        }

        headCode = packet[2];
        return IsGuildHeadCode(headCode);
    }

    public static bool IsGuildHeadCode(byte headCode) =>
        headCode is >= 0x50 and <= 0x56
            or 0x5D
            or >= 0x60 and <= 0x66
            or 0xE1
            or 0xE5
            or 0xE6
            or 0xEB
            or 0x67;

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

    /// <summary>Guild-list empty ack — client treats short C1 0x52 as empty roster.</summary>
    public static byte[] BuildEmptyGuildListAck() => BuildAck(0x52);

    /// <summary>Guild action failed — routes to <c>ReceiveGuildResult</c> chat, not modal UI.</summary>
    public static byte[] BuildGuildResultAck(byte result) => BuildAck(0x51, result);
}
