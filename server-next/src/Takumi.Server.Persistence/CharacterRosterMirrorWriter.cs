using System.Threading;

namespace Takumi.Server.Persistence;

/// <summary>Async Postgres mirror for <c>character_roster</c> (shared by Legacy + Game minimal hosts).</summary>
public static class CharacterRosterMirrorWriter
{
    private static int _pendingUpserts;

    public static int PendingUpsertsForTests => Volatile.Read(ref _pendingUpserts);

    public static void ScheduleReplaceAccountRoster(string accountId, IReadOnlyList<CharacterRosterRow> snapshot)
    {
        var repo = TakumiPostgresMirror.CharacterRoster;
        if (repo is null)
        {
            return;
        }

        CharacterRosterRow[] captured;
        try
        {
            captured = snapshot.Count == 0 ? Array.Empty<CharacterRosterRow>() : snapshot.ToArray();
        }
        catch
        {
            return;
        }

        Interlocked.Increment(ref _pendingUpserts);
        _ = Task.Run(
            async () =>
            {
                try
                {
                    await repo.ReplaceAccountRosterAsync(accountId, captured, CancellationToken.None).ConfigureAwait(false);
                    CharacterDomainMirrorWriter.ScheduleReplaceAccount(accountId, captured);
                    CharacterRosterMirrorHealth.RecordUpsertSuccess();
                }
                catch (Exception ex)
                {
                    CharacterRosterMirrorHealth.RecordUpsertFail();
                    Console.WriteLine(
                        "[roster-db] upsert failed for {0}: {1} | {2}",
                        accountId,
                        ex.Message,
                        CharacterRosterMirrorHealth.FormatSnapshot());
                }
                finally
                {
                    Interlocked.Decrement(ref _pendingUpserts);
                }
            });
    }

    public static void ScheduleVitalsUpsert(
        string accountId,
        string characterName,
        int currentHp,
        int maxHp,
        int currentMp,
        int maxMp,
        int currentShield = 0,
        int maxShield = 0)
    {
        var repo = TakumiPostgresMirror.CharacterRoster;
        if (repo is null || string.IsNullOrEmpty(accountId) || string.IsNullOrWhiteSpace(characterName))
        {
            return;
        }

        var name = CharacterRosterMerge.NormaliseName(characterName);
        Interlocked.Increment(ref _pendingUpserts);
        _ = Task.Run(
            async () =>
            {
                try
                {
                    await repo.UpsertVitalsAsync(accountId, name, currentHp, maxHp, currentMp, maxMp, currentShield, maxShield, CancellationToken.None)
                        .ConfigureAwait(false);
                    CharacterRosterMirrorHealth.RecordUpsertSuccess();
                }
                catch (Exception ex)
                {
                    CharacterRosterMirrorHealth.RecordUpsertFail();
                    Console.WriteLine("[roster-db] vitals upsert failed for {0}/{1}: {2}", accountId, name, ex.Message);
                }
                finally
                {
                    Interlocked.Decrement(ref _pendingUpserts);
                }
            });
    }

    /// <summary>Best-effort wait for fire-and-forget upserts (M4b shutdown / disconnect).</summary>
    public static void TryDrainPendingUpserts(TimeSpan maxWait)
    {
        var sw = System.Diagnostics.Stopwatch.StartNew();
        while (Volatile.Read(ref _pendingUpserts) > 0 && sw.Elapsed < maxWait)
        {
            Thread.Sleep(15);
        }

        CharacterRosterMirrorHealth.LogSnapshotIfEnabled();
    }
}
