using System.Collections.Concurrent;
using System.Threading;

namespace Takumi.Server.Persistence;

/// <summary>Async Postgres mirror for <c>character_roster</c> (shared by Legacy + Game minimal hosts).</summary>
public static class CharacterRosterMirrorWriter
{
    private static int _pendingUpserts;
    private static readonly ConcurrentDictionary<string, DebouncedProgressUpsert> ProgressDebounce = new();
    private static readonly TimeSpan ProgressDebounceDelay = TimeSpan.FromMilliseconds(300);

    public static int PendingUpsertsForTests => Volatile.Read(ref _pendingUpserts);

    private sealed class DebouncedProgressUpsert
    {
        public object Gate { get; } = new();
        public ProgressUpsertArgs? Latest;
        public int Generation;
        public CancellationTokenSource? DebounceCts;
    }

    private readonly record struct ProgressUpsertArgs(
        string AccountId,
        string CharacterName,
        ushort Level,
        uint Experience,
        ushort LevelUpPoint,
        int CurrentHp,
        int MaxHp,
        int CurrentMp,
        int MaxMp,
        int CurrentShield,
        int MaxShield,
        int Strength,
        int Dexterity,
        int Vitality,
        int Energy,
        int Leadership,
        int CurrentBp,
        int MaxBp);

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

    public static void ScheduleProgressUpsert(
        string accountId,
        string characterName,
        ushort level,
        uint experience,
        ushort levelUpPoint,
        int currentHp,
        int maxHp,
        int currentMp,
        int maxMp,
        int currentShield = 0,
        int maxShield = 0,
        int strength = 0,
        int dexterity = 0,
        int vitality = 0,
        int energy = 0,
        int leadership = 0,
        int currentBp = 0,
        int maxBp = 0)
    {
        var repo = TakumiPostgresMirror.CharacterRoster;
        if (repo is null || string.IsNullOrEmpty(accountId) || string.IsNullOrWhiteSpace(characterName))
        {
            return;
        }

        var name = CharacterRosterMerge.NormaliseName(characterName);
        var key = accountId + "\0" + name;
        var slot = ProgressDebounce.GetOrAdd(key, static _ => new DebouncedProgressUpsert());
        int generation;
        CancellationToken debounceToken;
        lock (slot.Gate)
        {
            slot.Latest = new ProgressUpsertArgs(
                accountId,
                name,
                level,
                experience,
                levelUpPoint,
                currentHp,
                maxHp,
                currentMp,
                maxMp,
                currentShield,
                maxShield,
                strength,
                dexterity,
                vitality,
                energy,
                leadership,
                currentBp,
                maxBp);
            slot.Generation++;
            generation = slot.Generation;
            slot.DebounceCts?.Cancel();
            slot.DebounceCts?.Dispose();
            slot.DebounceCts = new CancellationTokenSource();
            debounceToken = slot.DebounceCts.Token;
        }

        _ = RunDebouncedProgressUpsertAsync(slot, generation, debounceToken);
    }

    private static async Task RunDebouncedProgressUpsertAsync(
        DebouncedProgressUpsert slot,
        int generation,
        CancellationToken debounceToken)
    {
        try
        {
            await Task.Delay(ProgressDebounceDelay, debounceToken).ConfigureAwait(false);
        }
        catch (OperationCanceledException)
        {
            return;
        }

        ProgressUpsertArgs args;
        lock (slot.Gate)
        {
            if (generation != slot.Generation || slot.Latest is null)
            {
                return;
            }

            args = slot.Latest.Value;
        }

        await ExecuteProgressUpsertAsync(args).ConfigureAwait(false);
    }

    private static async Task ExecuteProgressUpsertAsync(ProgressUpsertArgs args)
    {
        var repo = TakumiPostgresMirror.CharacterRoster;
        if (repo is null)
        {
            return;
        }

        var exp = (long)Math.Min(args.Experience, long.MaxValue);
        Interlocked.Increment(ref _pendingUpserts);
        try
        {
            await repo.UpsertProgressAsync(
                    args.AccountId,
                    args.CharacterName,
                    args.Level,
                    exp,
                    args.LevelUpPoint,
                    args.CurrentHp,
                    args.MaxHp,
                    args.CurrentMp,
                    args.MaxMp,
                    args.CurrentShield,
                    args.MaxShield,
                    args.Strength,
                    args.Dexterity,
                    args.Vitality,
                    args.Energy,
                    args.Leadership,
                    args.CurrentBp,
                    args.MaxBp,
                    CancellationToken.None)
                .ConfigureAwait(false);
            if (CharacterDomainMirrorWriter.IsEnabled() && TakumiPostgresMirror.CharacterDomain is { } domainRepo)
            {
                await domainRepo.UpsertProgressAsync(
                        args.AccountId,
                        args.CharacterName,
                        args.Level,
                        exp,
                        args.LevelUpPoint,
                        args.CurrentHp,
                        args.MaxHp,
                        args.CurrentMp,
                        args.MaxMp,
                        args.CurrentShield,
                        args.MaxShield,
                        args.Strength,
                        args.Dexterity,
                        args.Vitality,
                        args.Energy,
                        args.Leadership,
                        args.CurrentBp,
                        args.MaxBp,
                        CancellationToken.None)
                    .ConfigureAwait(false);
            }

            CharacterRosterMirrorHealth.RecordUpsertSuccess();
        }
        catch (Exception ex)
        {
            CharacterRosterMirrorHealth.RecordUpsertFail();
            Console.WriteLine(
                "[roster-db] progress upsert failed for {0}/{1}: {2}",
                args.AccountId,
                args.CharacterName,
                ex.Message);
        }
        finally
        {
            Interlocked.Decrement(ref _pendingUpserts);
        }
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
