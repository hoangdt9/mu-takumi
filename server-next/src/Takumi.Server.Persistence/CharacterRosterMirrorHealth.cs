using System.Threading;

namespace Takumi.Server.Persistence;

/// <summary>M4b observability: counts for JSON↔Postgres roster merge (after login) vs async mirror upserts.</summary>
public static class CharacterRosterMirrorHealth
{
    private static long _mergeSuccess;
    private static long _mergeFail;
    private static long _upsertSuccess;
    private static long _upsertFail;

    public static long MergeSuccessCount => Interlocked.Read(ref _mergeSuccess);

    public static long MergeFailCount => Interlocked.Read(ref _mergeFail);

    public static long UpsertSuccessCount => Interlocked.Read(ref _upsertSuccess);

    public static long UpsertFailCount => Interlocked.Read(ref _upsertFail);

    public static bool IsHealthLogEnabled() =>
        string.Equals(Environment.GetEnvironmentVariable("TAKUMI_ROSTER_HEALTH_LOG"), "1", StringComparison.OrdinalIgnoreCase)
        || string.Equals(Environment.GetEnvironmentVariable("TAKUMI_ROSTER_HEALTH_LOG"), "true", StringComparison.OrdinalIgnoreCase);

    public static void RecordMergeSuccess() => Interlocked.Increment(ref _mergeSuccess);

    public static void RecordMergeFail() => Interlocked.Increment(ref _mergeFail);

    public static void RecordUpsertSuccess() => Interlocked.Increment(ref _upsertSuccess);

    public static void RecordUpsertFail() => Interlocked.Increment(ref _upsertFail);

    public static string FormatSnapshot() =>
        $"[roster-health] merge_ok={MergeSuccessCount} merge_fail={MergeFailCount} upsert_ok={UpsertSuccessCount} upsert_fail={UpsertFailCount}";

    /// <summary>Writes <see cref="FormatSnapshot"/> to stderr when <c>TAKUMI_ROSTER_HEALTH_LOG</c> is on.</summary>
    public static void LogSnapshotIfEnabled()
    {
        if (!IsHealthLogEnabled())
        {
            return;
        }

        Console.Error.WriteLine(FormatSnapshot());
    }

    /// <summary>Reset counters (unit tests only).</summary>
    public static void ResetCountersForTests()
    {
        Interlocked.Exchange(ref _mergeSuccess, 0);
        Interlocked.Exchange(ref _mergeFail, 0);
        Interlocked.Exchange(ref _upsertSuccess, 0);
        Interlocked.Exchange(ref _upsertFail, 0);
    }
}
