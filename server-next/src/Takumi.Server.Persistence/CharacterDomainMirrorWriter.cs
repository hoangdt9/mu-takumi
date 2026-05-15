namespace Takumi.Server.Persistence;

/// <summary>M4b: mirror <c>character_roster</c> snapshot into <c>character_domain</c> (same writer path).</summary>
public static class CharacterDomainMirrorWriter
{
    public static bool IsEnabled()
    {
        if (!CharacterRosterBootstrap.IsDbSyncEnabled())
        {
            return false;
        }

        var raw = Environment.GetEnvironmentVariable("TAKUMI_CHARACTER_DOMAIN_SYNC")?.Trim();
        return string.Equals(raw, "1", StringComparison.OrdinalIgnoreCase)
               || string.Equals(raw, "true", StringComparison.OrdinalIgnoreCase);
    }

    public static void ScheduleReplaceAccount(string accountId, IReadOnlyList<CharacterRosterRow> snapshot)
    {
        var repo = TakumiPostgresMirror.CharacterDomain;
        if (!IsEnabled() || repo is null)
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

        _ = Task.Run(
            async () =>
            {
                try
                {
                    await repo.ReplaceAccountAsync(accountId, captured, CancellationToken.None).ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("[character-domain] sync failed for {0}: {1}", accountId, ex.Message);
                }
            });
    }
}
