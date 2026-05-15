namespace Takumi.Server.Protocol;

/// <summary>Shop buy confirmation prompt (<c>C1 F3 ED</c>, parity <c>PMSG_ITEM_BUY_NEW</c>).</summary>
public static class ShopBuyConfirmWire602
{
    public const byte Head = 0xF3;
    public const byte Sub = 0xED;

    public static byte[] Build(byte shopSlot)
    {
        var buf = new byte[5];
        buf[0] = 0xC1;
        buf[1] = 5;
        buf[2] = Head;
        buf[3] = Sub;
        buf[4] = shopSlot;
        return buf;
    }
}
