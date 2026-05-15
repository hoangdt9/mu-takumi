namespace Takumi.Server.Protocol;

/// <summary><c>PMSG_NPC_TALK_SEND</c> — <c>C3 0x30</c> (encrypted header in legacy; plain <c>C1</c> for minimal host).</summary>
public static class NpcTalkWire602
{
    public const byte Head = 0x30;

    public static byte[] Build(byte result)
    {
        return [0xC1, 0x09, Head, result, 0, 0, 0, 0, 0, 0, 0];
    }
}
