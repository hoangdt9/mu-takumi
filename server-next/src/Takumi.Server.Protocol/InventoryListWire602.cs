using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>
/// Season 6 inventory list: <c>C4 F3 10</c> + <c>count</c> + repeated <c>slot (1)</c> + <c>ItemInfo[12]</c>.
/// Matches GameServer <c>PMSG_ITEM_LIST_SEND</c> / <c>GCItemListSend</c> (<c>ItemManager.cpp</c>) and client <c>ReceiveInventory</c> (<c>WSclient.cpp</c>).
/// </summary>
public static class InventoryListWire602
{
    public const int ItemWireBytes = 12;

    private const int HeaderWithCountLength = 6;

    /// <summary>Empty inventory (<c>count == 0</c>).</summary>
    public static byte[] BuildEmpty() => Build(ReadOnlySpan<byte>.Empty);

    /// <summary>
    /// Build from a flat payload: for each slot, <c>1 byte slot index</c> then <see cref="ItemWireBytes"/> item octets (same order as GameServer <c>PMSG_ITEM_LIST</c>).
    /// </summary>
    /// <param name="slotThenItem12Repeats">Length must be a multiple of <c>13</c>.</param>
    public static byte[] Build(ReadOnlySpan<byte> slotThenItem12Repeats)
    {
        if (slotThenItem12Repeats.Length % 13 != 0)
        {
            throw new ArgumentException("Length must be a multiple of 13 (slot + 12-byte item).", nameof(slotThenItem12Repeats));
        }

        var count = slotThenItem12Repeats.Length / 13;
        if (count > 255)
        {
            throw new ArgumentOutOfRangeException(nameof(slotThenItem12Repeats), "At most 255 items (count is a byte).");
        }

        var total = HeaderWithCountLength + slotThenItem12Repeats.Length;
        if (total > 65535)
        {
            throw new ArgumentOutOfRangeException(nameof(slotThenItem12Repeats), "Resulting packet length exceeds ushort.");
        }

        var p = new byte[total];
        p[0] = 0xC4;
        BinaryPrimitives.WriteUInt16BigEndian(p.AsSpan(1, 2), (ushort)total);
        p[3] = 0xF3;
        p[4] = 0x10;
        p[5] = (byte)count;
        slotThenItem12Repeats.CopyTo(p.AsSpan(HeaderWithCountLength));
        return p;
    }
}
