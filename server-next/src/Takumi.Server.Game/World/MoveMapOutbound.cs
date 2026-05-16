using MUnique.OpenMU.Network;
using Takumi.Server.Game.Networking;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Sends move-map checksum after character enters the world.</summary>
public static class MoveMapOutbound
{
    public static async Task TrySendChecksumAfterJoinAsync(
        Guid presenceSessionId,
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task>? writeAsync,
        CancellationToken ct)
    {
        if (MoveMapSessionState.SkipKeyCheck())
        {
            return;
        }

        var seed = MoveMapKeyGenerator.CreateSeed();
        MoveMapSessionState.Reset(presenceSessionId, seed);
        var packet = MoveMapWire602.BuildChecksum(seed);

        if (connection is not null)
        {
            await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, packet, ct).ConfigureAwait(false);
        }
        else if (writeAsync is not null)
        {
            await writeAsync(packet, ct).ConfigureAwait(false);
        }
        else
        {
            return;
        }

        Console.WriteLine("[m8] move map checksum seed=0x{0:X8} session={1}", seed, presenceSessionId);
    }
}
