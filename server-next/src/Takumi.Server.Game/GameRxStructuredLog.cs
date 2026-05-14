namespace Takumi.Server.Game;

/// <summary>Optional structured one-line logs for minimal game TCP (M6+).</summary>
public static class GameRxStructuredLog
{
    /// <summary>Force <c>[event=decrypted_rx]</c> on stderr even when <paramref name="verbose"/> is false.</summary>
    public static bool IsForced =>
        string.Equals(Environment.GetEnvironmentVariable("TAKUMI_STRUCTURED_LOG"), "1", StringComparison.OrdinalIgnoreCase)
        || string.Equals(Environment.GetEnvironmentVariable("TAKUMI_STRUCTURED_LOG"), "true", StringComparison.OrdinalIgnoreCase);

    public static void DecryptedRx(string remote, int decryptedLength, byte headCode, bool verbose)
    {
        if (verbose || IsForced)
        {
            Console.Error.WriteLine(
                "[event=decrypted_rx] remote={0} len={1} head=0x{2:X2}",
                remote,
                decryptedLength,
                headCode);
            return;
        }

        Console.WriteLine("[{0}] decrypted len={1} head=0x{2:X2}", remote, decryptedLength, headCode);
    }
}
