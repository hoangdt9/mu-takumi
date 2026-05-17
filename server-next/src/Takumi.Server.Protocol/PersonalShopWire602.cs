namespace Takumi.Server.Protocol;

/// <summary>Personal shop packets (<c>0x3F</c> subcodes, parity <c>CPersonalShop</c>).</summary>
public static class PersonalShopWire602
{
    public const byte HeadCode = 0x3F;

    /// <summary><c>C2 3F 00</c> shop title viewport (count=0 clears client redraw state).</summary>
    public static byte[] BuildViewportClear() =>
    [
        0xC2, 0x00, 0x06, HeadCode, 0x00, 0x00,
    ];

    /// <summary><c>C1 3F 02</c> open result (S→C).</summary>
    public static byte[] BuildOpenResult(byte result) =>
    [
        0xC1, 0x05, HeadCode, 0x02, result,
    ];

    /// <summary><c>C1 3F 03</c> close result (S→C).</summary>
    public static byte[] BuildCloseResult(byte result, ushort objectIndex = 0) =>
    [
        0xC1, 0x07, HeadCode, 0x03, result,
        (byte)((objectIndex >> 8) & 0xFF),
        (byte)(objectIndex & 0xFF),
    ];
}
