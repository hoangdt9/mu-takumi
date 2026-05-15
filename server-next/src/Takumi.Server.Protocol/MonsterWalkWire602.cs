namespace Takumi.Server.Protocol;

/// <summary>Monster walk (<c>PMSG_MOVE_SEND</c> / <c>GCViewport</c> move, head <c>0xD4</c>).</summary>
public static class MonsterWalkWire602
{
    public const byte HeadCode = 0xD4;

    /// <summary>Season 6 client: object id + target tile (7 bytes, same layout as <see cref="PlayerPositionWire602"/>).</summary>
    public static byte[] Build(int objectKey, byte targetX, byte targetY)
    {
        var buf = new byte[7];
        buf[0] = 0xC1;
        buf[1] = 7;
        buf[2] = HeadCode;
        var k = objectKey & 0x7FFF;
        buf[3] = (byte)((k >> 8) & 0xFF);
        buf[4] = (byte)(k & 0xFF);
        buf[5] = targetX;
        buf[6] = targetY;
        return buf;
    }
}
