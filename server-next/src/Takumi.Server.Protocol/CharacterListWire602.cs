using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>
/// Character list <c>C1 F3 00</c> with Takumi <c>PRECEIVE_CHARACTER_LIST</c> entries (WSclient.h).
/// MSVC layout uses 1 padding byte after <c>ID[10]</c> before <c>WORD Level</c> → 34 bytes per slot.
/// </summary>
public static class CharacterListWire602
{
    /// <summary>PMSG_CHARACTER_LIST_SEND with ExtWarehouse (Season 6+ layout, 8 bytes total).</summary>
    public static byte[] BuildEmpty() => new byte[] { 0xC1, 0x08, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00 };

    public static byte[] Build(IReadOnlyList<CharacterRosterWire> roster)
    {
        const int headerSize = 8;
        const int entrySize = 34;
        var n = roster.Count;
        var total = headerSize + n * entrySize;
        var p = new byte[total];
        p[0] = 0xC1;
        p[1] = (byte)total;
        p[2] = 0xF3;
        p[3] = 0x00;
        p[4] = 7; // MaxClass
        p[5] = 0; // MoveCount
        p[6] = (byte)n;
        p[7] = 0; // ExtWarehouse

        var off = headerSize;
        for (var i = 0; i < n; i++)
        {
            var e = roster[i];
            p[off++] = (byte)i;
            e.Name10.AsSpan(0, 10).CopyTo(p.AsSpan(off));
            off += 10;
            p[off++] = 0; // struct padding before Level (matches MSVC PRECEIVE_CHARACTER_LIST)
            BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(off), e.Level);
            off += 2;
            p[off++] = 0; // CtlCode
            p[off++] = e.ServerClass;
            for (var k = 0; k < 17; k++)
            {
                p[off++] = 0xFF;
            }

            // G_NONE = (BYTE)-1
            p[off++] = 0xFF;
        }

        return p;
    }
}
