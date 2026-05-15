namespace Takumi.Server.Protocol;

/// <summary>
/// Season 6 <c>C2 0x12</c> player viewport (parity <c>GCViewportPlayerSend</c> /
/// <c>ReceiveCreatePlayerViewport</c> — <c>PCREATE_CHARACTER</c> with <c>s_BuffCount=0</c>).
/// </summary>
public static class PlayerViewportWire602
{
    public const byte HeadCode = 0x12;

    /// <summary>Wire bytes per entry when <c>s_BuffCount</c> is 0 (Takumi client with <c>HAISLOTRING</c>).</summary>
    public const int EntrySizeZeroBuffs = 38;

    public static byte[] Build(IReadOnlyList<PlayerViewportEntry> entries)
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
            WriteEntry(buf.AsSpan(o, EntrySizeZeroBuffs), e);
            o += EntrySizeZeroBuffs;
        }

        return buf;
    }

    static void WriteEntry(Span<byte> entry, PlayerViewportEntry e)
    {
        var key = e.ObjectKey & 0x7FFF;
        var keyH = (byte)((key >> 8) & 0xFF);
        var keyL = (byte)(key & 0xFF);
        if (e.CreateFlag)
        {
            keyH |= 0x80;
        }

        entry[0] = keyH;
        entry[1] = keyL;
        entry[2] = e.X;
        entry[3] = e.Y;
        entry[4] = e.ServerClass;
        entry.Slice(5, 17).Fill(0xFF);
        var nameLen = Math.Min(10, e.Name10.Length);
        e.Name10.AsSpan(0, nameLen).CopyTo(entry.Slice(22, nameLen));
        entry[32] = e.TargetX;
        entry[33] = e.TargetY;
        entry[34] = (byte)(((Math.Clamp(e.Angle, (byte)1, (byte)16) - 1) & 0x0F) << 4 | (e.PkLevel & 0x0F));
        entry[35] = 0xFF;
        entry[36] = 0xFF;
        entry[37] = 0;
    }
}

public readonly record struct PlayerViewportEntry(
    int ObjectKey,
    byte X,
    byte Y,
    byte TargetX,
    byte TargetY,
    byte ServerClass,
    byte[] Name10,
    byte Angle,
    byte PkLevel = 0,
    bool CreateFlag = true);
