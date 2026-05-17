using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Tracks personal shop open state for move-map blocks (<c>PShopOpen</c>).</summary>
public static class PersonalShopGameplayHandler
{
    public static async Task<bool> TryHandlePacketAsync(
        Guid presenceSessionId,
        byte[] packet,
        string remote,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        CancellationToken ct)
    {
        if (ClientGameplayPackets602.TryFindPersonalShopOpenRequest(packet, out _))
        {
            PlayerUiSession.SetPersonalShop(presenceSessionId, true);
            await writeAsync(PersonalShopWire602.BuildOpenResult(1), ct).ConfigureAwait(false);
            Console.WriteLine("[m8] personal shop open {0}", remote);
            return true;
        }

        if (ClientGameplayPackets602.TryFindPersonalShopCloseRequest(packet, out _))
        {
            PlayerUiSession.SetPersonalShop(presenceSessionId, false);
            await writeAsync(PersonalShopWire602.BuildCloseResult(1), ct).ConfigureAwait(false);
            Console.WriteLine("[m8] personal shop close {0}", remote);
            return true;
        }

        return false;
    }

    public static Task SendViewportRedrawAsync(
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        CancellationToken ct) =>
        writeAsync(PersonalShopWire602.BuildViewportClear(), ct);
}
