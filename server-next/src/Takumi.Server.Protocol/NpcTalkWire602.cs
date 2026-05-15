namespace Takumi.Server.Protocol;

/// <summary>NPC talk open shop (<c>PMSG_NPC_TALK_SEND</c> / head <c>0x30</c>, <c>result=0</c> → client <c>INTERFACE_NPCSHOP</c>).</summary>
public static class NpcTalkWire602
{
    public const byte HeadCode = 0x30;

    public const int PacketLength = 11;

    public static byte[] BuildShopOpen()
    {
        var buf = new byte[PacketLength];
        buf[0] = 0xC1;
        buf[1] = PacketLength;
        buf[2] = HeadCode;
        buf[3] = 0;
        return buf;
    }
}
