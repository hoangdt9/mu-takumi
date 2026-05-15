using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary><c>GCItemValueSend</c> — <c>C2 F3 E9</c> with <c>ITEM_VALUE_DATA</c> rows.</summary>
public static class ItemValueWire602
{
    public const byte Head = 0xF3;
    public const byte Sub = 0xE9;
    public const int EntryBytes = 28;

    public readonly record struct ItemValueEntry(
        int Index,
        int Level,
        int NewOpt,
        int Type,
        int Value,
        int BuySell,
        int SellValue);

    public static byte[] Build(IReadOnlyList<ItemValueEntry> entries)
    {
        if (entries.Count == 0)
        {
            return Array.Empty<byte>();
        }

        var size = 9 + entries.Count * EntryBytes;
        var buf = new byte[size];
        buf[0] = 0xC2;
        buf[1] = (byte)((size >> 8) & 0xFF);
        buf[2] = (byte)(size & 0xFF);
        buf[3] = Head;
        buf[4] = Sub;
        BinaryPrimitives.WriteInt32LittleEndian(buf.AsSpan(5), entries.Count);

        var o = 9;
        foreach (var e in entries)
        {
            BinaryPrimitives.WriteInt32LittleEndian(buf.AsSpan(o), e.Index);
            BinaryPrimitives.WriteInt32LittleEndian(buf.AsSpan(o + 4), e.Level);
            BinaryPrimitives.WriteInt32LittleEndian(buf.AsSpan(o + 8), e.NewOpt);
            BinaryPrimitives.WriteInt32LittleEndian(buf.AsSpan(o + 12), e.Type);
            BinaryPrimitives.WriteInt32LittleEndian(buf.AsSpan(o + 16), e.Value);
            BinaryPrimitives.WriteInt32LittleEndian(buf.AsSpan(o + 20), e.BuySell);
            BinaryPrimitives.WriteInt32LittleEndian(buf.AsSpan(o + 24), e.SellValue);
            o += EntryBytes;
        }

        return buf;
    }
}
