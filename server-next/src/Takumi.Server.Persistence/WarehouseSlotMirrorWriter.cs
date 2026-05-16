using System.Threading;

namespace Takumi.Server.Persistence;

/// <summary>Async Postgres mirror for <c>warehouse_slot</c>.</summary>
public static class WarehouseSlotMirrorWriter
{
    private static int _pendingOps;

    public static void ScheduleUpsertSlot(string accountLogin, byte slot, byte[] item12)
    {
        var repo = TakumiPostgresMirror.WarehouseSlots;
        if (repo is null || item12.Length != 12 || string.IsNullOrEmpty(accountLogin))
        {
            return;
        }

        var account = accountLogin;
        var slotCopy = slot;
        var blob = item12.ToArray();
        Enqueue(
            async () => await repo.UpsertSlotAsync(account, slotCopy, blob, CancellationToken.None).ConfigureAwait(false),
            "upsert",
            $"{account} slot={slotCopy}");
    }

    public static void ScheduleDeleteSlot(string accountLogin, byte slot)
    {
        var repo = TakumiPostgresMirror.WarehouseSlots;
        if (repo is null || string.IsNullOrEmpty(accountLogin))
        {
            return;
        }

        var account = accountLogin;
        var slotCopy = slot;
        Enqueue(
            async () => await repo.DeleteSlotAsync(account, slotCopy, CancellationToken.None).ConfigureAwait(false),
            "delete",
            $"{account} slot={slotCopy}");
    }

    public static void ScheduleReplaceAccount(string accountLogin, IReadOnlyList<InventorySlotRow> snapshot)
    {
        var repo = TakumiPostgresMirror.WarehouseSlots;
        if (repo is null || string.IsNullOrEmpty(accountLogin))
        {
            return;
        }

        var account = accountLogin;
        InventorySlotRow[] captured;
        try
        {
            captured = snapshot.Count == 0 ? Array.Empty<InventorySlotRow>() : snapshot.ToArray();
        }
        catch
        {
            return;
        }

        Enqueue(
            async () => await repo.ReplaceAccountSlotsAsync(account, captured, CancellationToken.None).ConfigureAwait(false),
            "replace",
            $"{account} count={captured.Length}");
    }

    public static void TryDrainPendingOps(TimeSpan maxWait)
    {
        var sw = System.Diagnostics.Stopwatch.StartNew();
        while (Volatile.Read(ref _pendingOps) > 0 && sw.Elapsed < maxWait)
        {
            Thread.Sleep(15);
        }
    }

    static void Enqueue(Func<Task> work, string op, string tag)
    {
        Interlocked.Increment(ref _pendingOps);
        _ = Task.Run(
            async () =>
            {
                try
                {
                    await work().ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("[warehouse-db] {0} failed {1}: {2}", op, tag, ex.Message);
                }
                finally
                {
                    Interlocked.Decrement(ref _pendingOps);
                }
            });
    }
}
