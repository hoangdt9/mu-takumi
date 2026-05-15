namespace Takumi.Server.Protocol;

/// <summary>Monster death + exp stub (<c>GCMonsterDieSend</c> / <c>ReceiveDieExp</c>, head <c>0x16</c>).</summary>
public static class MonsterDieWire602
{
    public const byte HeadCode = 0x16;

    public static byte[] Build(int targetObjectKey, ushort experience, int damage, bool dieSuccess = true)
    {
        const int size = 14;
        var buf = new byte[size];
        buf[0] = 0xC1;
        buf[1] = (byte)size;
        buf[2] = HeadCode;

        var key = targetObjectKey & 0x7FFF;
        if (dieSuccess)
        {
            key |= 0x8000;
        }

        buf[3] = (byte)((key >> 8) & 0xFF);
        buf[4] = (byte)(key & 0xFF);
        buf[5] = (byte)((experience >> 8) & 0xFF);
        buf[6] = (byte)(experience & 0xFF);
        buf[7] = (byte)((damage >> 8) & 0xFF);
        buf[8] = (byte)(damage & 0xFF);
        WriteUInt32Le(buf, 9, (uint)Math.Max(0, damage));
        return buf;
    }

    static void WriteUInt32Le(byte[] buf, int offset, uint value)
    {
        buf[offset] = (byte)(value & 0xFF);
        buf[offset + 1] = (byte)((value >> 8) & 0xFF);
        buf[offset + 2] = (byte)((value >> 16) & 0xFF);
        buf[offset + 3] = (byte)((value >> 24) & 0xFF);
    }
}
