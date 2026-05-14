namespace Takumi.Server.Game.Networking;

/// <summary>Minimal Season6 join packet (C1 F1 00) after TCP accept — parity with GameServer GCConnectClientSend path.</summary>
public static class GameJoinWire
{
    public static byte[] BuildJoinPacket(byte result, ushort index, ReadOnlySpan<byte> version5)
    {
        if (version5.Length != 5)
        {
            throw new ArgumentException("Join version must be exactly 5 bytes.", nameof(version5));
        }

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
}
