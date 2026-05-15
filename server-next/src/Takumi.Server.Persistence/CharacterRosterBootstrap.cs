namespace Takumi.Server.Persistence;

/// <summary>M4b: optional Postgres-first roster load (<c>TAKUMI_ROSTER_DB_PRIMARY</c>).</summary>
public static class CharacterRosterBootstrap
{
    public static bool IsDbPrimaryEnabled()
    {
        if (!IsDbSyncEnabled())
        {
            return false;
        }

        var raw = Environment.GetEnvironmentVariable("TAKUMI_ROSTER_DB_PRIMARY")?.Trim();
        return string.Equals(raw, "1", StringComparison.OrdinalIgnoreCase)
               || string.Equals(raw, "true", StringComparison.OrdinalIgnoreCase);
    }

    public static bool IsDbSyncEnabled()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC")?.Trim();
        return string.Equals(raw, "1", StringComparison.OrdinalIgnoreCase)
               || string.Equals(raw, "true", StringComparison.OrdinalIgnoreCase);
    }

    /// <summary>When primary mode is on, skip writing JSON on save (Postgres upsert still runs).</summary>
    public static bool ShouldSkipJsonExportOnSave()
    {
        if (!IsDbPrimaryEnabled())
        {
            return false;
        }

        var raw = Environment.GetEnvironmentVariable("TAKUMI_ROSTER_JSON_EXPORT")?.Trim();
        if (string.Equals(raw, "1", StringComparison.OrdinalIgnoreCase)
            || string.Equals(raw, "true", StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }

        return true;
    }

    /// <summary>
    /// Loads roster from Postgres when primary mode is enabled and rows exist.
    /// Returns true when <paramref name="target"/> was filled from DB (caller should skip JSON load + overlay).
    /// </summary>
    public static async Task<bool> TryLoadDbPrimaryAsync(
        string accountId,
        List<CharacterRosterRow> target,
        CancellationToken ct = default)
    {
        target.Clear();
        if (!IsDbPrimaryEnabled() || TakumiPostgresMirror.CharacterRoster is not { } repo)
        {
            return false;
        }

        try
        {
            var rows = await repo.LoadByAccountAsync(accountId, ct).ConfigureAwait(false);
            if (rows.Count == 0 && TakumiPostgresMirror.CharacterDomain is { } domainRepo)
            {
                rows = await domainRepo.LoadByAccountAsync(accountId, ct).ConfigureAwait(false);
                if (rows.Count > 0)
                {
                    Console.WriteLine(
                        "[roster] db-primary: loaded {0} character(s) from character_domain for account={1}",
                        rows.Count,
                        accountId);
                }
            }

            if (rows.Count == 0)
            {
                Console.WriteLine(
                    "[roster] db-primary: no rows for account={0} — falling back to JSON",
                    accountId);
                return false;
            }

            target.AddRange(rows);
            Console.WriteLine(
                "[roster] db-primary: loaded {0} character(s) for account={1}",
                target.Count,
                accountId);
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine("[roster] db-primary load failed for {0}: {1}", accountId, ex.Message);
            return false;
        }
    }

    public static void ApplyDbOverlayToRows<T>(
        IReadOnlyList<CharacterRosterRow> dbRows,
        IReadOnlyList<T> roster,
        Func<T, string> getName,
        Action<T, CharacterRosterRow> applyFields)
    {
        CharacterRosterMerge.ApplyDbOverlay(roster, dbRows, getName, applyFields);
    }
}
