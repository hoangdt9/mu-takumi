using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Warehouse zen deposit/withdraw (<c>C1 0x81</c>) while vault UI is open.</summary>
public static class WarehouseGameplayHandler
{
    public static async Task<bool> TryHandlePacketAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] packet,
        string remote,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        CancellationToken ct)
    {
        if (!WarehouseWire602.TryFindStorageGoldRequest(packet, out _, out var flag, out var gold))
        {
            return false;
        }

        if (!PlayerWarehouseSession.IsOpen(presenceSessionId))
        {
            await writeAsync(WarehouseWire602.BuildMoney(0, 0, result: 0), ct).ConfigureAwait(false);
            Console.WriteLine("[warehouse] 0x81 rejected vault closed {0}", remote);
            return true;
        }

        await AccountWalletSession.EnsureLoadedAsync(accountId, ct).ConfigureAwait(false);

        if (!AccountWalletSession.TryTransferWarehouseZen(accountId, player, flag, gold, out var invZen, out var whZen))
        {
            await writeAsync(WarehouseWire602.BuildMoney((uint)player.Zen, (uint)AccountWalletSession.GetWarehouseZen(accountId), result: 0), ct)
                .ConfigureAwait(false);
            Console.WriteLine("[warehouse] 0x81 transfer fail flag={0} gold={1} {2}", flag, gold, remote);
            return true;
        }

        onRosterDirty?.Invoke();
        await writeAsync(WarehouseWire602.BuildMoney(invZen, whZen), ct).ConfigureAwait(false);
        Console.WriteLine(
            "[warehouse] 0x81 flag={0} amount={1} inv={2} wh={3} {4}",
            flag,
            gold,
            invZen,
            whZen,
            remote);
        return true;
    }
}
