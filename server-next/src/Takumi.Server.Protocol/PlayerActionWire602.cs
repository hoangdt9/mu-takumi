namespace Takumi.Server.Protocol;

/// <summary>Combat action broadcast (<c>GCActionSend</c> / <c>PMSG_ACTION_SEND</c> head <c>0x18</c>).</summary>
public static class PlayerActionWire602
{
    public const byte HeadCode = 0x18;

    public static byte[] Build(int attackerKey, byte dir, byte action, int targetKey)
    {
        var buf = new byte[9];
        buf[0] = 0xC1;
        buf[1] = 9;
        buf[2] = HeadCode;
        var a = attackerKey & 0x7FFF;
        buf[3] = (byte)((a >> 8) & 0xFF);
        buf[4] = (byte)(a & 0xFF);
        buf[5] = dir;
        buf[6] = action;
        var t = targetKey & 0x7FFF;
        buf[7] = (byte)((t >> 8) & 0xFF);
        buf[8] = (byte)(t & 0xFF);
        return buf;
    }
}
