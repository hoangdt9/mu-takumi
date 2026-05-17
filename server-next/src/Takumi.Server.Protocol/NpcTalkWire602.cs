namespace Takumi.Server.Protocol;

/// <summary><c>PMSG_NPC_TALK_SEND</c> — <c>C3 0x30</c> (encrypted header in legacy; plain <c>C1</c> for minimal host).</summary>
public static class NpcTalkWire602
{
    public const byte Head = 0x30;

    /// <summary>Legacy <c>C1 0x30</c> is 9 bytes total; extra trailing bytes desync Android recv when shop <c>C2 0x31</c> follows in the same read.</summary>
    public static byte[] Build(byte result)
    {
        return [0xC1, 0x09, Head, result, 0, 0, 0, 0, 0];
    }
}
