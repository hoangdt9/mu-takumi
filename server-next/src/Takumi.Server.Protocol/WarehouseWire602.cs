using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>Warehouse open stubs (<c>GCWarehouseMoneySend</c> / <c>GCWarehouseStateSend</c>).</summary>
public static class WarehouseWire602
{
    public const byte MoneyHead = 0x81;
    /// <summary>Client <c>SendRequestStorageExit</c>: <c>C1 03 82</c>.</summary>
    public const byte ExitHead = 0x82;
    public const byte StateHead = 0x83;

    /// <summary><c>PMSG_WAREHOUSE_MONEY_SEND</c> — inventory + warehouse zen after open.</summary>
    public static byte[] BuildMoney(uint inventoryMoney, uint warehouseMoney, byte result = 1)
    {
        var buf = new byte[12];
        buf[0] = 0xC1;
        buf[1] = 12;
        buf[2] = MoneyHead;
        buf[3] = result;
        BinaryPrimitives.WriteUInt32LittleEndian(buf.AsSpan(4), warehouseMoney);
        BinaryPrimitives.WriteUInt32LittleEndian(buf.AsSpan(8), inventoryMoney);
        return buf;
    }

    /// <summary>Client <c>SendRequestStorageGold</c>: <c>C1 len 0x81 flag gold:dword</c>.</summary>
    public static bool TryFindStorageGoldRequest(
        ReadOnlySpan<byte> packet,
        out int frameOffset,
        out byte flag,
        out uint gold)
    {
        frameOffset = 0;
        flag = 0;
        gold = 0;
        for (var i = 0; i + 7 <= packet.Length; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 2] != MoneyHead)
            {
                continue;
            }

            var len = packet[i + 1];
            if (len < 7 || i + len > packet.Length)
            {
                continue;
            }

            frameOffset = i;
            flag = packet[i + 3];
            gold = BinaryPrimitives.ReadUInt32LittleEndian(packet.Slice(i + 4, 4));
            return true;
        }

        return false;
    }

    /// <summary>Client closes vault UI (<c>C1 len 0x82</c>).</summary>
    public static bool TryFindStorageExitRequest(ReadOnlySpan<byte> packet, out int frameOffset)
    {
        frameOffset = -1;
        for (var i = 0; i <= packet.Length - 3; i++)
        {
            if (packet[i] == 0xC1 && packet[i + 1] == 0x03 && packet[i + 2] == ExitHead)
            {
                frameOffset = i;
                return true;
            }
        }

        return false;
    }

    /// <summary><c>PMSG_WAREHOUSE_STATE_SEND</c> — 0 = unlocked, no password.</summary>
    public static byte[] BuildState(byte state = 0)
    {
        var buf = new byte[5];
        buf[0] = 0xC1;
        buf[1] = 5;
        buf[2] = StateHead;
        buf[3] = state;
        return buf;
    }
}
