namespace Takumi.Server.Protocol;

/// <summary>Join-server ack <c>C1 F1 00</c> and login result <c>C1 F1 01</c>.</summary>
public static class LoginAccountWire602
{
    public static byte[] BuildJoinPacket(byte result, ushort index, ReadOnlySpan<byte> version5)
    {
        var p = new byte[12];
        p[0] = 0xC1;
        p[1] = 12;
        p[2] = 0xF1;
        p[3] = 0x00;
        p[4] = result;
        p[5] = (byte)((index >> 8) & 0xFF);
        p[6] = (byte)(index & 0xFF);
        version5.CopyTo(p.AsSpan(7));
        return p;
    }

    /// <summary><c>PMSG_CONNECT_ACCOUNT_SEND</c>: PSBMSG_HEAD + result byte.</summary>
    public static ReadOnlyMemory<byte> BuildLoginResult(byte result) =>
        new byte[] { 0xC1, 0x05, 0xF1, 0x01, result };
}
