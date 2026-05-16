namespace Takumi.Client.Protocol;

/// <summary>Minimal framing parse for inbound <c>C1</c> packets (pre-decrypt or post-decrypt).</summary>
public static class ClientPacketEnvelope
{
    public static bool TryReadC1(ReadOnlySpan<byte> buffer, out byte length, out byte headCode, out byte subCode)
    {
        length = 0;
        headCode = 0;
        subCode = 0;
        if (buffer.Length < 4 || buffer[0] != 0xC1)
        {
            return false;
        }

        length = buffer[1];
        if (length < 4 || buffer.Length < length)
        {
            return false;
        }

        headCode = buffer[2];
        subCode = buffer[3];
        return true;
    }
}
