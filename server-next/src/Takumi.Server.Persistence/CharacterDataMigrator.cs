namespace Takumi.Server.Persistence;

/// <summary>
/// M7 unified migrate entry: Postgres-first (staging + optional JSON backfill).
/// Runtime SSOT is <c>character_roster</c> / <c>inventory_slot</c> — JSON is optional dev cache only.
/// </summary>
public static class CharacterDataMigrator
{
    public readonly record struct MigrateAllSummary(
        CharacterRosterJsonMigrator.MigrateSummary? RosterJson,
        InventoryStagingImporter.ImportSummary InventoryStaging,
        int CharactersInDb);

    public static bool IsMigrateOnlyMode() =>
        CharacterRosterJsonMigrator.IsMigrateOnlyMode()
        || string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_MIGRATE_CHARACTER_DATA_ONLY")?.Trim(),
            "1",
            StringComparison.OrdinalIgnoreCase)
        || string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_MIGRATE_CHARACTER_DATA_ONLY")?.Trim(),
            "true",
            StringComparison.OrdinalIgnoreCase);

    public static bool ShouldMigrateRosterJson() =>
        CharacterRosterJsonMigrator.IsMigrateOnStartupEnabled()
        || CharacterRosterJsonMigrator.IsMigrateOnlyMode()
        || string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_MIGRATE_ROSTER_JSON")?.Trim(),
            "1",
            StringComparison.OrdinalIgnoreCase);

    public static async Task<MigrateAllSummary> MigrateAllAsync(CancellationToken ct = default)
    {
        CharacterRosterJsonMigrator.MigrateSummary? rosterJson = null;
        if (ShouldMigrateRosterJson())
        {
            rosterJson = await CharacterRosterJsonMigrator.MigrateAllJsonFilesAsync(ct).ConfigureAwait(false);
        }
        else
        {
            Console.WriteLine(
                "[character-migrate] roster JSON skipped (SSOT=Postgres). Set TAKUMI_MIGRATE_ROSTER_JSON=1 for one-time backfill from takumi-roster/*.json");
        }

        var inventory = await InventoryStagingImporter.TryImportAsync(ct).ConfigureAwait(false);
        var keys = await CharacterRosterDiscovery.ListCharactersFromPostgresAsync(ct).ConfigureAwait(false);
        Console.WriteLine("[character-migrate] Postgres character keys={0}", keys.Count);
        return new MigrateAllSummary(rosterJson, inventory, keys.Count);
    }
}
