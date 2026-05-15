namespace Takumi.Server.Protocol;

/// <summary>Season 6 <c>GCUserDieSend</c> / client <c>ReceiveDie</c> (<c>C1 0x17</c>).</summary>
public static class PlayerDieWire602
{
    public const byte HeadCode = 0x17;
    public const int PacketLength = 9;

    public static byte[] Build(int victimObjectKey, int killerObjectKey = 0, int skill = 0)
    {
        var buf = new byte[PacketLength];
        buf[0] = 0xC1;
        buf[1] = PacketLength;
        buf[2] = HeadCode;
        var v = victimObjectKey & 0xFFFF;
        buf[3] = (byte)((v >> 8) & 0xFF);
        buf[4] = (byte)(v & 0xFF);
        var s = skill & 0xFFFF;
        buf[5] = (byte)((s >> 8) & 0xFF);
        buf[6] = (byte)(s & 0xFF);
        var k = killerObjectKey & 0xFFFF;
        buf[7] = (byte)((k >> 8) & 0xFF);
        buf[8] = (byte)(k & 0xFF);
        return buf;
    }
}
