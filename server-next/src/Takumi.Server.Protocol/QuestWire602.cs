namespace Takumi.Server.Protocol;

/// <summary>Legacy quest packets (<c>GCQuestInfoSend</c> <c>C1 A0</c>, <c>GCQuestStateSend</c> <c>C1 A1</c>).</summary>
public static class QuestWire602
{
    public static byte[] BuildQuestInfo(IReadOnlyList<byte>? questStates = null)
    {
        var buf = new byte[52];
        buf[0] = 0xC1;
        buf[1] = 52;
        buf[2] = 0xA0;
        buf[3] = 50;
        var states = questStates ?? Array.Empty<byte>();
        for (var i = 0; i < 50; i++)
        {
            buf[4 + i] = i < states.Count ? states[i] : (byte)0xFF;
        }

        return buf;
    }

    public static byte[] BuildQuestState(byte questIndex, byte questState) =>
        [0xC1, 0x05, 0xA1, questIndex, questState];
}
