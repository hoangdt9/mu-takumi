using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Warehouse zen (<c>C1 0x81</c>) and close (<c>C1 0x82</c>).</summary>
public static class WarehouseGameplayHandler
{
    /// <summary>Close vault server-side (persist + clear UI flag). Used for <c>0x82</c> and stale warehouse before move-map.</summary>
    public static void CloseVaultIfOpen(Guid presenceSessionId, string? accountId, string reason, string remote)
    {
        if (!PlayerWarehouseSession.IsOpen(presenceSessionId))
        {
            return;
        }

        var snapshot = PlayerWarehouseSession.BuildSnapshot(presenceSessionId);
        if (snapshot.Count > 0 && !string.IsNullOrEmpty(accountId))
        {
            WarehouseSlotMirrorWriter.ScheduleReplaceAccount(accountId, snapshot);
        }

        PlayerWarehouseSession.Close(presenceSessionId);
        Console.WriteLine("[warehouse] vault closed reason={0} {1}", reason, remote);
    }

    public static Task<bool> TryHandlePacketAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] packet,
        string remote,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        CancellationToken ct)
    {
        if (WarehouseWire602.TryFindStorageExitRequest(packet, out _))
        {
            CloseVaultIfOpen(presenceSessionId, accountId, "0x82", remote);
            return Task.FromResult(true);
        }

        return TryHandleStorageGoldAsync(
            player,
            presenceSessionId,
            accountId,
            packet,
            remote,
            writeAsync,
            onRosterDirty,
            ct);
    }

    static async Task<bool> TryHandleStorageGoldAsync(
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
