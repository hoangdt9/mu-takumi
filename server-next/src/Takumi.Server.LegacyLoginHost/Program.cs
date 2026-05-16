// Combined Connect (F4) + login/game TCP — same as legacy single-process Docker service.
using Takumi.Server.Hosting;
using Takumi.Server.LegacyLoginHost;
using Takumi.Server.Persistence;

RepoEnvLoader.ApplyDefaultsAndLocalEnv();
DockerRuntimeEnv.ApplyStackOverridesIfEnabled();
TakumiPostgresMirror.InitIfEnabled();

if (InventorySlotJsonMigrator.IsMigrateOnlyMode())
{
    await InventorySlotJsonMigrator.MigrateAllJsonFilesAsync().ConfigureAwait(false);
    return 0;
}

if (CharacterRosterJsonMigrator.IsMigrateOnlyMode())
{
    await CharacterRosterJsonMigrator.MigrateAllJsonFilesAsync().ConfigureAwait(false);
    return 0;
}

if (CharacterRosterJsonMigrator.IsMigrateOnStartupEnabled())
{
    await CharacterRosterJsonMigrator.MigrateAllJsonFilesAsync().ConfigureAwait(false);
}

if (InventorySlotJsonMigrator.IsMigrateOnStartupEnabled())
{
    await InventorySlotJsonMigrator.MigrateAllJsonFilesAsync().ConfigureAwait(false);
}

using var cts = new CancellationTokenSource();
Console.CancelKeyPress += (_, e) => { e.Cancel = true; cts.Cancel(); };

return await LegacyLoginHostRunner.RunAsync(
    new LegacyLoginHostRunOptions(LegacyLoginHostListenMode.RespectEnvironment),
    cts.Token).ConfigureAwait(false);
