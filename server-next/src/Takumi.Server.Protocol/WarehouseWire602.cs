using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>Warehouse open stubs (<c>GCWarehouseMoneySend</c> / <c>GCWarehouseStateSend</c>).</summary>
public static class WarehouseWire602
{
    public const byte MoneyHead = 0x81;
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
