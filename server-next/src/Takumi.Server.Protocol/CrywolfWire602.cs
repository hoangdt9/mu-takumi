namespace Takumi.Server.Protocol;

/// <summary>Crywolf event packets (<c>GC 0xBD</c>).</summary>
public static class CrywolfWire602
{
    public const byte HeadCode = 0xBD;
    public const byte SubInfo = 0x00;

    /// <summary>Peaceful idle — no MVP result overlay on map enter.</summary>
    public const byte OccupationPeace = 0;
    public const byte StateNone = 0;

    /// <summary><c>PMSG_ANS_CRYWOLF_INFO</c>: <c>C1 06 BD 00</c> + occupation + state.</summary>
    public static byte[] BuildInfo(byte occupationState, byte crywolfState) =>
        new byte[] { 0xC1, 0x06, HeadCode, SubInfo, occupationState, crywolfState };

    public static bool TryFindInfoRequest(ReadOnlySpan<byte> packet, out int frameOffset)
    {
        frameOffset = -1;
        for (var i = 0; i + 4 <= packet.Length; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 2] != HeadCode || packet[i + 3] != SubInfo)
            {
                continue;
            }

            var len = packet[i + 1];
            if (len < 4 || i + len > packet.Length)
            {
                continue;
            }

            frameOffset = i;
            return true;
        }

        return false;
    }
}
