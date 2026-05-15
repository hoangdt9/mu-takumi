using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>NPC shop commerce (<c>0x32</c> buy, <c>0x33</c> sell, <c>0x34</c> repair).</summary>
public static class ShopCommerceWire602
{
    public const byte BuyHead = 0x32;
    public const byte SellHead = 0x33;
    public const byte RepairHead = 0x34;

    public const byte BuyFailIndex = 0xFF;
    public const byte BuyShopClosedIndex = 0xFE;

    /// <summary><c>PHEADER_DEFAULT_ITEM</c> — Index + Item[12].</summary>
    public static byte[] BuildBuy(byte inventorySlot, ReadOnlySpan<byte> item12)
    {
        var buf = new byte[16];
        buf[0] = 0xC1;
        buf[1] = 16;
        buf[2] = BuyHead;
        buf[3] = inventorySlot;
        item12.CopyTo(buf.AsSpan(4, ItemWire602.WireBytes));
        return buf;
    }

    public static byte[] BuildBuyFail(byte index = BuyFailIndex) =>
        BuildBuy(index, stackalloc byte[ItemWire602.WireBytes]);

    /// <summary><c>PRECEIVE_GOLD</c> layout (Flag + Gold).</summary>
    public static byte[] BuildSell(byte flag, uint gold)
    {
        var buf = new byte[9];
        buf[0] = 0xC1;
        buf[1] = 9;
        buf[2] = SellHead;
        buf[3] = flag;
        BinaryPrimitives.WriteUInt32LittleEndian(buf.AsSpan(4), gold);
        return buf;
    }

    /// <summary><c>PRECEIVE_REPAIR_GOLD</c>.</summary>
    public static byte[] BuildRepair(uint gold)
    {
        var buf = new byte[7];
        buf[0] = 0xC1;
        buf[1] = 7;
        buf[2] = RepairHead;
        BinaryPrimitives.WriteUInt32LittleEndian(buf.AsSpan(3), gold);
        return buf;
    }
}
