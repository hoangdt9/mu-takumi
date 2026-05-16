namespace Takumi.Server.Protocol;

/// <summary>Move-map packets (<c>GC 0x8E</c> subs <c>0x01</c> / <c>0x03</c>).</summary>
public static class MoveMapWire602
{
    public const byte HeadCode = 0x8E;
    public const byte SubChecksum = 0x01;
    public const byte SubAnswer = 0x03;

    public const byte ResultSuccess = 0x01;
    public const byte ResultFailed = 0x00;
    public const byte ResultNotEnoughZen = 0x07;
    public const byte ResultNotEnoughLevel = 0x08;

    /// <summary><c>C1 08 8E 01</c> + little-endian seed (<c>PMSG_MAPMOVE_CHECKSUM</c>).</summary>
    public static byte[] BuildChecksum(uint keyValue)
    {
        var packet = new byte[8];
        packet[0] = 0xC1;
        packet[1] = 0x08;
        packet[2] = HeadCode;
        packet[3] = SubChecksum;
        packet[4] = (byte)(keyValue & 0xFF);
        packet[5] = (byte)((keyValue >> 8) & 0xFF);
        packet[6] = (byte)((keyValue >> 16) & 0xFF);
        packet[7] = (byte)((keyValue >> 24) & 0xFF);
        return packet;
    }

    public static byte[] BuildAnswer(byte result) => new byte[] { 0xC1, 0x05, HeadCode, SubAnswer, result };
}
