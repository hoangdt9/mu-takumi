namespace Takumi.Server.Protocol;

/// <summary>Move-map responses (<c>GC 0x8E</c> sub <c>0x03</c>).</summary>
public static class MoveMapWire602
{
    public const byte HeadCode = 0x8E;
    public const byte SubAnswer = 0x03;

    public const byte ResultSuccess = 0x01;
    public const byte ResultFailed = 0x00;
    public const byte ResultNotEnoughZen = 0x07;
    public const byte ResultNotEnoughLevel = 0x08;

    public static byte[] BuildAnswer(byte result) => new byte[] { 0xC1, 0x05, HeadCode, SubAnswer, result };
}
