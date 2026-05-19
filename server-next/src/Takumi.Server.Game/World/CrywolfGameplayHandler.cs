using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Minimal Crywolf info stub so map 34 clients do not keep stale MVP overlay state.</summary>
public static class CrywolfGameplayHandler
{
    public static Task<bool> TryHandlePacketAsync(
        byte[] packet,
        string remote,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        CancellationToken ct)
    {
        if (!CrywolfWire602.TryFindInfoRequest(packet, out _))
        {
            return Task.FromResult(false);
        }

        return SendIdleInfoAsync(writeAsync, remote, ct);
    }

    static async Task<bool> SendIdleInfoAsync(
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        string remote,
        CancellationToken ct)
    {
        await writeAsync(
                CrywolfWire602.BuildInfo(CrywolfWire602.OccupationPeace, CrywolfWire602.StateNone),
                ct)
            .ConfigureAwait(false);
        Console.WriteLine("[m8] crywolf info stub peace/none {0}", remote);
        return true;
    }
}
