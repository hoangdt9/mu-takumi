using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Guild packets stub — swallow or safe acks until full guild domain exists.</summary>
public static class GuildGameplayHandler
{
    /// <summary>Result bytes for <c>ReceiveGuildResult</c> (client GlobalText 503+).</summary>
    private const byte GuildResultGenericFail = 0;
    private const byte GuildResultAlreadyMember = 1;

    public static async Task<bool> TryHandlePacketAsync(
        byte[] packet,
        string remote,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        CancellationToken ct)
    {
        if (!GuildWire602.TryParseLeadingC1Frame(packet, out var head, out _))
        {
            return false;
        }

        switch (head)
        {
            // Client → server: invite answer — no server reply (echoing 0x51 opens error spam).
            case 0x51:
            // Client → server: guild-war accept/decline — NEVER echo 0x61 (reopens war dialog loop).
            case 0x61:
            // Server → client only; if seen inbound, swallow silently.
            case 0x62:
            case 0x63:
            case 0x64:
                Console.WriteLine("[m11] guild request swallowed 0x{0:X2} {1}", head, remote);
                return true;

            // Client → server: invite request — reply with result chat, not 0x50 modal.
            case 0x50:
                await writeAsync(GuildWire602.BuildGuildResultAck(GuildResultAlreadyMember), ct).ConfigureAwait(false);
                Console.WriteLine("[m11] guild invite rejected (stub) 0x50→0x51 result={0} {1}", GuildResultAlreadyMember, remote);
                return true;

            // Client → server: roster list — empty list is safe.
            case 0x52:
                await writeAsync(GuildWire602.BuildEmptyGuildListAck(), ct).ConfigureAwait(false);
                Console.WriteLine("[m11] guild empty list ack 0x52 {0}", remote);
                return true;

            // Client → server: declare war — do not reply 0x61 (opens undeclarable war UI).
            case 0x60:
                await writeAsync(GuildWire602.BuildGuildResultAck(GuildResultGenericFail), ct).ConfigureAwait(false);
                Console.WriteLine("[m11] guild declare-war rejected (stub) 0x60→0x51 {0}", remote);
                return true;

            default:
                await writeAsync(GuildWire602.BuildAck(head), ct).ConfigureAwait(false);
                Console.WriteLine("[m11] guild stub ack 0x{0:X2} {1}", head, remote);
                return true;
        }
    }
}
