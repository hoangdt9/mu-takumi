using Takumi.Server.Game.Networking;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Player trade (<c>0x36</c> request, <c>0x37</c> response, <c>0x3D</c> exit).</summary>
public static class TradeGameplayHandler
{
    public static async Task<bool> TryHandlePacketAsync(
        Guid presenceSessionId,
        byte[] characterName10,
        byte[] packet,
        string remote,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        CancellationToken ct)
    {
        if (TradeWire602.TryFindTradeExit(packet, out _))
        {
            PlayerTradeSession.Close(presenceSessionId);
            await writeAsync(TradeWire602.BuildTradeExit(), ct).ConfigureAwait(false);
            Console.WriteLine("[m11] trade exit {0}", remote);
            return true;
        }

        if (TradeWire602.TryFindTradeResponse(packet, out _, out var accept))
        {
            if (accept == 0)
            {
                PlayerTradeSession.Close(presenceSessionId);
                await writeAsync(TradeWire602.BuildTradeResult(0), ct).ConfigureAwait(false);
                return true;
            }

            if (!Sessions.TryGetPendingRequester(presenceSessionId, out var partnerSession))
            {
                await writeAsync(TradeWire602.BuildTradeResponse(0, "unknown"), ct).ConfigureAwait(false);
                return true;
            }

            PlayerTradeSession.OpenPair(presenceSessionId, partnerSession);
            Sessions.ClearPending(presenceSessionId);
            var name = PlayerTradeSession.ReadName10(characterName10);
            await writeAsync(TradeWire602.BuildTradeResponse(1, name, level: 1), ct).ConfigureAwait(false);
            if (GameMapPresenceRegistry.TryGetSession(partnerSession, out var partner) && partner is not null)
            {
                await GamePortOutboundWire.WriteAsync(
                        partner.Connection,
                        partner.Protect,
                        TradeWire602.BuildTradeResponse(1, name, level: 1),
                        ct)
                    .ConfigureAwait(false);
            }

            Console.WriteLine("[m11] trade open {0}", remote);
            return true;
        }

        if (TradeWire602.TryFindTradeRequest(packet, out _, out var targetIndex))
        {
            if (!GameMapPresenceRegistry.TryGetByObjectKey(targetIndex, out var target)
                || target is null
                || target.SessionId == presenceSessionId)
            {
                await writeAsync(TradeWire602.BuildTradeResponse(0, "offline"), ct).ConfigureAwait(false);
                return true;
            }

            var requesterName = PlayerTradeSession.ReadName10(characterName10);
            Sessions.SetPending(target.SessionId, presenceSessionId);
            await GamePortOutboundWire.WriteAsync(
                    target.Connection,
                    target.Protect,
                    TradeWire602.BuildTradeRequest(requesterName),
                    ct)
                .ConfigureAwait(false);
            Console.WriteLine("[m11] trade request {0} → key={1}", remote, targetIndex);
            return true;
        }

        return false;
    }

    static class Sessions
    {
        static readonly System.Collections.Concurrent.ConcurrentDictionary<Guid, Guid> PendingByTarget = new();

        public static void SetPending(Guid targetSessionId, Guid requesterSessionId) =>
            PendingByTarget[targetSessionId] = requesterSessionId;

        public static bool TryGetPendingRequester(Guid targetSessionId, out Guid requesterSessionId) =>
            PendingByTarget.TryGetValue(targetSessionId, out requesterSessionId);

        public static void ClearPending(Guid targetSessionId) =>
            PendingByTarget.TryRemove(targetSessionId, out _);
    }
}
