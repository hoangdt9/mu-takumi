using System.Threading;

namespace Takumi.Server.Persistence;

/// <summary>Async Postgres mirror for <c>inventory_slot</c> (shop buy/sell/repair + disconnect flush).</summary>
public static class InventorySlotMirrorWriter
{
    private static int _pendingOps;

    public static int PendingOpsForTests => Volatile.Read(ref _pendingOps);

    public static void ScheduleUpsertSlot(string accountLogin, string characterName, byte slot, byte[] item12)
    {
        var repo = TakumiPostgresMirror.InventorySlots;
        if (repo is null || item12.Length != 12)
        {
            return;
        }

        var account = accountLogin;
        var name = CharacterRosterMerge.NormaliseName(characterName);
        var slotCopy = slot;
        var blob = item12.ToArray();
        Enqueue(
            async () =>
            {
                await repo.UpsertSlotAsync(account, name, slotCopy, blob, CancellationToken.None).ConfigureAwait(false);
            },
            "upsert",
            $"{account}/{name} slot={slotCopy}");
    }

    public static void ScheduleDeleteSlot(string accountLogin, string characterName, byte slot)
    {
        var repo = TakumiPostgresMirror.InventorySlots;
        if (repo is null)
        {
            return;
        }

        var account = accountLogin;
        var name = CharacterRosterMerge.NormaliseName(characterName);
        var slotCopy = slot;
        Enqueue(
            async () =>
            {
                await repo.DeleteSlotAsync(account, name, slotCopy, CancellationToken.None).ConfigureAwait(false);
            },
            "delete",
            $"{account}/{name} slot={slotCopy}");
    }

    public static void ScheduleReplaceCharacter(
        string accountLogin,
        string characterName,
        IReadOnlyList<InventorySlotRow> snapshot)
    {
        var repo = TakumiPostgresMirror.InventorySlots;
        if (repo is null)
        {
            return;
        }

        var account = accountLogin;
        var name = CharacterRosterMerge.NormaliseName(characterName);
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
            async () =>
            {
                await repo.ReplaceCharacterSlotsAsync(account, name, captured, CancellationToken.None).ConfigureAwait(false);
            },
            "replace",
            $"{account}/{name} count={captured.Length}");
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
                    Console.WriteLine("[inventory-db] {0} failed {1}: {2}", op, tag, ex.Message);
                }
                finally
                {
                    Interlocked.Decrement(ref _pendingOps);
                }
            });
    }

    public static void TryDrainPendingOps(TimeSpan maxWait)
    {
        var sw = System.Diagnostics.Stopwatch.StartNew();
        while (Volatile.Read(ref _pendingOps) > 0 && sw.Elapsed < maxWait)
        {
            Thread.Sleep(15);
        }
    }
}
