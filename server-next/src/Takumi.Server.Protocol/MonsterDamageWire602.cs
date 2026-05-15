namespace Takumi.Server.Protocol;

/// <summary>Season 6 damage packet (<c>GCDamageSend</c> / <c>ReceiveAttackDamage</c>, head <c>0x11</c>).</summary>
public static class MonsterDamageWire602
{
    public const byte HeadCode = 0x11;

    /// <summary>Builds <c>PRECEIVE_ATTACK</c> with QWORD view damage (Takumi <c>FixDmgQWORD</c>).</summary>
    public static byte[] Build(int targetObjectKey, int damage, int viewCurHp, bool hitSuccess, byte damageType = 0)
    {
        const int size = 37;
        var buf = new byte[size];
        buf[0] = 0xC1;
        buf[1] = (byte)size;
        buf[2] = HeadCode;

        var key = targetObjectKey & 0x7FFF;
        if (hitSuccess)
        {
            key |= 0x8000;
        }

        buf[3] = (byte)((key >> 8) & 0xFF);
        buf[4] = (byte)(key & 0xFF);
        buf[5] = (byte)((damage >> 8) & 0xFF);
        buf[6] = (byte)(damage & 0xFF);
        buf[7] = damageType;
        buf[8] = 0;
        buf[9] = 0;

        WriteUInt32Le(buf, 10, (uint)Math.Max(0, viewCurHp));
        WriteUInt32Le(buf, 14, 0);
        WriteUInt64Le(buf, 18, (ulong)Math.Max(0, damage));
        WriteUInt64Le(buf, 26, 0);
        return buf;
    }

    static void WriteUInt32Le(byte[] buf, int offset, uint value)
    {
        buf[offset] = (byte)(value & 0xFF);
        buf[offset + 1] = (byte)((value >> 8) & 0xFF);
        buf[offset + 2] = (byte)((value >> 16) & 0xFF);
        buf[offset + 3] = (byte)((value >> 24) & 0xFF);
    }

    static void WriteUInt64Le(byte[] buf, int offset, ulong value)
    {
        for (var i = 0; i < 8; i++)
        {
            buf[offset + i] = (byte)((value >> (8 * i)) & 0xFF);
        }
    }
}
