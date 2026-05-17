namespace Takumi.Server.Protocol;

/// <summary><c>CG [0xB2][0x1A]</c> — client <c>ReceiveTaxInfo</c> (shop tax rate).</summary>
public static class NpcShopTaxWire602
{
    public const byte Head = 0xB2;
    public const byte Sub = 0x1A;
    public const byte TaxTypeNpcShop = 2;

    public static byte[] Build(byte taxRatePercent)
    {
        var buf = new byte[6];
        buf[0] = 0xC1;
        buf[1] = 6;
        buf[2] = Head;
        buf[3] = Sub;
        buf[4] = TaxTypeNpcShop;
        buf[5] = (byte)Math.Clamp((int)taxRatePercent, 0, 100);
        return buf;
    }
}
