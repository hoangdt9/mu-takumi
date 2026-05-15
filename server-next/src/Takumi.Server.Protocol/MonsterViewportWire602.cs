namespace Takumi.Server.Protocol;

/// <summary>Season 6 <c>C2 0x13</c> monster viewport (parity <c>GCViewportMonsterSend</c> / <c>ReceiveCreateMonsterViewport</c>).</summary>
public static class MonsterViewportWire602
{
    public const byte HeadCode = 0x13;
    public const int EntrySizeZeroBuffs = 10;

    public static byte[] Build(IReadOnlyList<MonsterViewportEntry> entries)
    {
        if (entries.Count == 0)
        {
            return Array.Empty<byte>();
        }

        var size = 4 + 1 + (entries.Count * EntrySizeZeroBuffs);
        var buf = new byte[size];
        buf[0] = 0xC2;
        buf[1] = (byte)((size >> 8) & 0xFF);
        buf[2] = (byte)(size & 0xFF);
        buf[3] = HeadCode;
        buf[4] = (byte)entries.Count;

        var o = 5;
        foreach (var e in entries)
        {
            var key = e.ObjectKey & 0x7FFF;
            if (e.CreateFlag)
            {
                key |= 0x8000;
            }

            if (e.TeleportFlag)
            {
                key |= 0x4000;
            }

            buf[o++] = (byte)((key >> 8) & 0xFF);
            buf[o++] = (byte)(key & 0xFF);
            buf[o++] = (byte)((e.MonsterClass >> 8) & 0x03);
            buf[o++] = (byte)(e.MonsterClass & 0xFF);
            buf[o++] = e.X;
            buf[o++] = e.Y;
            buf[o++] = e.TargetX;
            buf[o++] = e.TargetY;
            buf[o++] = (byte)((e.Dir << 4) & 0xF0);
            buf[o++] = 0;
        }

        return buf;
    }
}

public readonly record struct MonsterViewportEntry(
    int ObjectKey,
    int MonsterClass,
    byte X,
    byte Y,
    byte TargetX,
    byte TargetY,
    byte Dir,
    bool CreateFlag = true,
    bool TeleportFlag = false);
