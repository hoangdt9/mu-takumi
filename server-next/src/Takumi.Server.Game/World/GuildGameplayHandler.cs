using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Guild packets stub — ack with empty result until full guild domain exists.</summary>
public static class GuildGameplayHandler
{
    public static async Task<bool> TryHandlePacketAsync(
        byte[] packet,
        string remote,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        CancellationToken ct)
    {
        if (!GuildWire602.TryFindGuildPacket(packet, out var head))
        {
            return false;
        }

        await writeAsync(GuildWire602.BuildAck(head), ct).ConfigureAwait(false);
        Console.WriteLine("[m11] guild stub ack 0x{0:X2} {1}", head, remote);
        return true;
    }
}
