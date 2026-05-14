using System.Buffers.Binary;
using System.Security.Cryptography;
using System.Text;

namespace Takumi.Server.Protocol;

/// <summary>HMAC-SHA256 over a fixed v1 layout for <see cref="SessionTicketWire602"/>.</summary>
public static class SessionTicketSignature602
{
    public const int AccountWireBytes = 10;

    public const int MacLength = 32;

    /// <summary>First MAC byte context (server push / echoed attach).</summary>
    public const byte MacContextV1 = 0xA5;

    public static ReadOnlySpan<byte> ResolveHmacKeyFromEnv()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_SESSION_TICKET_HMAC_KEY")?.Trim();
        if (string.IsNullOrEmpty(raw) || Encoding.UTF8.GetByteCount(raw) < 8)
        {
            return ReadOnlySpan<byte>.Empty;
        }

        return Encoding.UTF8.GetBytes(raw);
    }

    public static byte[] ComputeMacV1(ReadOnlySpan<byte> hmacKey, Guid ticketId, long expiresUnixUtc, ReadOnlySpan<byte> account10Wire)
    {
        if (account10Wire.Length != AccountWireBytes)
        {
            throw new ArgumentException("account10Wire must be 10 bytes.", nameof(account10Wire));
        }

        Span<byte> msg = stackalloc byte[1 + 16 + 8 + AccountWireBytes];
        msg[0] = MacContextV1;
        ticketId.TryWriteBytes(msg.Slice(1, 16));
        BinaryPrimitives.WriteInt64BigEndian(msg.Slice(17, 8), expiresUnixUtc);
        account10Wire.CopyTo(msg.Slice(25, 10));
        return HMACSHA256.HashData(hmacKey, msg);
    }

    public static bool FixedTimeEquals(ReadOnlySpan<byte> a, ReadOnlySpan<byte> b) =>
        a.Length == b.Length && CryptographicOperations.FixedTimeEquals(a, b);

    /// <summary>ASCII account padded/truncated to 10 (same padding style as login id field).</summary>
    public static void FormatAccount10(string accountLogin, Span<byte> dest10)
    {
        dest10.Clear();
        if (string.IsNullOrEmpty(accountLogin))
        {
            return;
        }

        var i = 0;
        foreach (var c in accountLogin.Trim())
        {
            if (i >= AccountWireBytes)
            {
                break;
            }

            if (c < 128)
            {
                dest10[i++] = (byte)c;
            }
        }
    }
}