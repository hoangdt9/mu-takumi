namespace Takumi.Server.Game;

/// <summary>Optional structured one-line logs for minimal game TCP (M6+).</summary>
public static class GameRxStructuredLog
{
    /// <summary>Force <c>[event=decrypted_rx]</c> on stderr even when <paramref name="verbose"/> is false.</summary>
    public static bool IsForced =>
        string.Equals(Environment.GetEnvironmentVariable("TAKUMI_STRUCTURED_LOG"), "1", StringComparison.OrdinalIgnoreCase)
        || string.Equals(Environment.GetEnvironmentVariable("TAKUMI_STRUCTURED_LOG"), "true", StringComparison.OrdinalIgnoreCase);

    public static void DecryptedRx(string remote, ReadOnlySpan<byte> packet, bool verbose)
    {
        var headCode = packet.Length > 0 ? packet[0] : (byte)0;
        var subCode = packet.Length > 2 ? packet[2] : (byte)0;
        if (verbose || IsForced)
        {
            Console.Error.WriteLine(
                packet.Length is >= 3 and <= 24
                    ? "[event=decrypted_rx] remote={0} len={1} head=0x{2:X2} sub=0x{3:X2}"
                    : "[event=decrypted_rx] remote={0} len={1} head=0x{2:X2}",
                remote,
                packet.Length,
                headCode,
                subCode);
            return;
        }

        Console.WriteLine("[{0}] decrypted len={1} head=0x{2:X2}", remote, packet.Length, headCode);
    }
}
