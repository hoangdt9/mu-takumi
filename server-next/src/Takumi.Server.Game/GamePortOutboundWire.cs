using MUnique.OpenMU.Network;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>Game TCP outbound: optional client <c>gProtect</c> wire layer (Android <c>ENCRYPT_STATE</c> + GS port range).</summary>
public static class GamePortOutboundWire
{
    public static async Task WriteAsync(
        Connection connection,
        (byte K1, byte K2)? clientProtectOutbound,
        ReadOnlyMemory<byte> pkt,
        CancellationToken ct,
        Action<ReadOnlySpan<byte>>? trackOutbound = null)
    {
        trackOutbound?.Invoke(pkt.Span);
        if (clientProtectOutbound is { } k)
        {
            var tmp = new byte[pkt.Length];
            pkt.CopyTo(tmp);
            TakumiClientProtectWire602.EncryptInPlace(tmp, k.K1, k.K2);
            await connection.Output.WriteAsync(tmp, ct).ConfigureAwait(false);
        }
        else
        {
            await connection.Output.WriteAsync(pkt, ct).ConfigureAwait(false);
        }

        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
    }
}
