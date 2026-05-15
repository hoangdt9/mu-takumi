using System.Text;

namespace Takumi.Server.Protocol;

/// <summary>
/// Byte obfuscation used by Takumi Android when <c>ENCRYPT_STATE==1</c> and the peer port is in
/// <c>gProtect.m_MainInfo.GSPortMin..GSPortMax</c> (see <c>Protect.cpp</c> / <c>android_link_stubs.cpp</c>).
/// Outbound server bytes must be transformed with <see cref="EncryptInPlace"/> so the client's
/// <c>DecryptData</c> on recv restores the MU packet (e.g. plain <c>C1 F1 00</c> join).
/// </summary>
public static class TakumiClientProtectWire602
{
    public const byte BaseEncDecKey1 = 0xF1;

    public const byte BaseEncDecKey2 = 0x1A;

    /// <summary>Matches <c>CProtect::LoadEncDec</c> loop over <c>CustomerName[32]</c> and <c>ClientSerial[17]</c>.</summary>
    public static (byte EncDecKey1, byte EncDecKey2) DeriveKeys(string customerName, ReadOnlySpan<byte> clientSerial16Ascii)
    {
        Span<byte> cn = stackalloc byte[32];
        Span<byte> cs = stackalloc byte[17];
        cn.Clear();
        cs.Clear();
        var nameSpan = customerName.AsSpan();
        if (nameSpan.Length > 32)
        {
            nameSpan = nameSpan[..32];
        }

        Encoding.ASCII.GetBytes(nameSpan, cn);
        if (clientSerial16Ascii.Length >= 16)
        {
            clientSerial16Ascii[..16].CopyTo(cs);
        }
        else
        {
            clientSerial16Ascii.CopyTo(cs);
        }

        ushort acc = 0;
        for (var n = 0; n < 32; n++)
        {
            acc += (byte)(cn[n] ^ cs[n % 17]);
        }

        return (
            (byte)(BaseEncDecKey1 + unchecked((byte)(acc & 0xFF))),
            (byte)(BaseEncDecKey2 + unchecked((byte)(acc >> 8))));
    }

    /// <summary>Matches <c>CProtect::EncryptData</c> (client send / server outbound on GS TCP).</summary>
    public static void EncryptInPlace(Span<byte> data, byte encDecKey1, byte encDecKey2)
    {
        for (var n = 0; n < data.Length; n++)
        {
            data[n] += encDecKey2;
            data[n] ^= encDecKey1;
        }
    }

    /// <summary>Matches <c>CProtect::DecryptData</c> (client recv) — for tests / diagnostics.</summary>
    public static void DecryptInPlace(Span<byte> data, byte encDecKey1, byte encDecKey2)
    {
        for (var n = 0; n < data.Length; n++)
        {
            data[n] ^= encDecKey1;
            data[n] -= encDecKey2;
        }
    }
}
