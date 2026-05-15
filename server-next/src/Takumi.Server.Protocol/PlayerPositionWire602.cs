namespace Takumi.Server.Protocol;

/// <summary>Player tile update (<c>PMSG_POSITION_SEND</c> / head <c>0x15</c>, GAMESERVER_LANGUAGE 1).</summary>
public static class PlayerPositionWire602
{
    public const byte HeadCode = 0x15;

    public static byte[] Build(int objectKey, byte x, byte y)
    {
        var buf = new byte[7];
        buf[0] = 0xC1;
        buf[1] = 7;
        buf[2] = HeadCode;
        var k = objectKey & 0x7FFF;
        buf[3] = (byte)((k >> 8) & 0xFF);
        buf[4] = (byte)(k & 0xFF);
        buf[5] = x;
        buf[6] = y;
        return buf;
    }
}
