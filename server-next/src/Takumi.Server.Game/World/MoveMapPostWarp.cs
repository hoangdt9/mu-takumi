using MUnique.OpenMU.Network;
using Takumi.Server.Game.Networking;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Post-warp client hooks (parity <c>PShopRedrawAbs</c>).</summary>
internal static class MoveMapPostWarp
{
    public static async Task SendPersonalShopViewportRedrawAsync(
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        CancellationToken ct)
    {
        var pkt = PersonalShopWire602.BuildViewportClear();
        if (connection is not null)
        {
            await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, pkt, ct).ConfigureAwait(false);
            return;
        }

        await writeAsync(pkt, ct).ConfigureAwait(false);
    }
}
