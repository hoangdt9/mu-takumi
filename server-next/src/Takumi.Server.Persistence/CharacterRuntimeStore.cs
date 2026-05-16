namespace Takumi.Server.Persistence;

/// <summary>
/// Runtime character access for Game hosts — maps <c>character_roster</c> rows (minimal-host SSOT)
/// until full <c>Takumi.Server.Host</c> EF <c>takumi_runtime.character</c> is wired.
/// </summary>
public static class CharacterRuntimeStore
{
    public static async Task<CharacterRosterRow?> TryLoadCharacterAsync(
        string accountLogin,
        string characterName,
        CancellationToken ct = default)
    {
        if (!CharacterRosterBootstrap.IsDbSyncEnabled() || TakumiPostgresMirror.CharacterRoster is not { } repo)
        {
            return null;
        }

        var name = CharacterRosterMerge.NormaliseName(characterName);
        var rows = await repo.LoadByAccountAsync(accountLogin, ct).ConfigureAwait(false);
        foreach (var row in rows)
        {
            if (string.Equals(row.Name, name, StringComparison.OrdinalIgnoreCase))
            {
                return row;
            }
        }

        if (TakumiPostgresMirror.CharacterDomain is { } domain)
        {
            rows = await domain.LoadByAccountAsync(accountLogin, ct).ConfigureAwait(false);
            foreach (var row in rows)
            {
                if (string.Equals(row.Name, name, StringComparison.OrdinalIgnoreCase))
                {
                    return row;
                }
            }
        }

        return null;
    }
}
