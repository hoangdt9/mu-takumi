namespace Takumi.Server.Protocol;

/// <summary>Season 6 item pick/drop/move responses (<c>0x22</c>–<c>0x24</c>).</summary>
public static class ItemWorldWire602
{
    public const byte HeadPick = 0x22;
    public const byte HeadDrop = 0x23;
    public const byte HeadMove = 0x24;
    public const byte HeadItemDelete = 0x28;
    public const byte HeadItemDur = 0x2A;

    public const byte PickFail = 0xFF;
    public const byte PickZen = 0xFE;
    public const byte PickStack = 0xFD;
    public const byte MoveFail = 0xFF;

    public const int MovePacketLength = 17;

    /// <summary><c>PRECEIVE_GET_ITEM</c> — C1/C3 with result + item[12].</summary>
    public static byte[] BuildPick(byte result, ReadOnlySpan<byte> item12)
    {
        var buf = new byte[16];
        buf[0] = 0xC1;
        buf[1] = 16;
        buf[2] = HeadPick;
        buf[3] = result;
        item12.CopyTo(buf.AsSpan(4, ItemWire602.WireBytes));
        return buf;
    }

    public static byte[] BuildPickFail() =>
        BuildPick(PickFail, stackalloc byte[ItemWire602.WireBytes]);

    /// <summary>C1 0x22 / result 0xFE — absolute inventory zen (legacy <c>ReceiveGetItem</c> / OpenMU InventoryMoneyUpdate).</summary>
    public static byte[] BuildInventoryMoneyUpdate(uint zen)
    {
        var zenWire = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteZenPickTotal(zenWire, zen);
        return BuildPick(PickZen, zenWire);
    }

    /// <summary><c>PHEADER_DEFAULT_KEY</c> — KeyH=success, KeyL=slot.</summary>
    public static byte[] BuildDrop(byte keyH, byte slot)
    {
        var buf = new byte[5];
        buf[0] = 0xC1;
        buf[1] = 5;
        buf[2] = HeadDrop;
        buf[3] = keyH;
        buf[4] = slot;
        return buf;
    }

    public static byte[] BuildDropSuccess(byte slot) => BuildDrop(1, slot);

    public static byte[] BuildDropFail(byte slot) => BuildDrop(0, slot);

    /// <summary><c>PHEADER_DEFAULT_SUBCODE_ITEM</c> — SubCode=result, Index=slot.</summary>
    public static byte[] BuildMove(byte subCode, byte targetSlot, ReadOnlySpan<byte> item12)
    {
        var buf = new byte[MovePacketLength];
        buf[0] = 0xC1;
        buf[1] = MovePacketLength;
        buf[2] = HeadMove;
        buf[3] = subCode;
        buf[4] = targetSlot;
        item12.CopyTo(buf.AsSpan(5, ItemWire602.WireBytes));
        return buf;
    }

    public static byte[] BuildMoveFail(byte targetSlot)
    {
        var buf = new byte[MovePacketLength];
        buf[0] = 0xC1;
        buf[1] = MovePacketLength;
        buf[2] = HeadMove;
        buf[3] = MoveFail;
        buf[4] = targetSlot;
        for (var i = 0; i < ItemWire602.WireBytes; i++)
        {
            buf[5 + i] = 0xFF;
        }

        return buf;
    }

    /// <param name="storageSubCode">Target storage flag sent as <c>SubCode</c> (<see cref="ItemStorageFlags602"/>).</param>
    public static byte[] BuildMoveSuccess(byte storageSubCode, byte targetSlot, ReadOnlySpan<byte> item12) =>
        BuildMove(storageSubCode, targetSlot, item12);

    /// <summary><c>GCItemDeleteSend</c> — C1:28 slot + flag.</summary>
    public static byte[] BuildItemDelete(byte slot, byte flag = 1)
    {
        var buf = new byte[5];
        buf[0] = 0xC1;
        buf[1] = 5;
        buf[2] = HeadItemDelete;
        buf[3] = slot;
        buf[4] = flag;
        return buf;
    }

    /// <summary><c>GCItemDurSend</c> — C1:2A slot + durability + flag.</summary>
    public static byte[] BuildItemDur(byte slot, byte durability, byte flag = 0)
    {
        var buf = new byte[6];
        buf[0] = 0xC1;
        buf[1] = 6;
        buf[2] = HeadItemDur;
        buf[3] = slot;
        buf[4] = durability;
        buf[5] = flag;
        return buf;
    }
}
