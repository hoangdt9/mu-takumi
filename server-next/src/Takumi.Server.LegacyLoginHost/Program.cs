// Combined Connect (F4) + login/game TCP — same as legacy single-process Docker service.
using Takumi.Server.Hosting;
using Takumi.Server.LegacyLoginHost;

RepoEnvLoader.ApplyDefaultsAndLocalEnv();

using var cts = new CancellationTokenSource();
Console.CancelKeyPress += (_, e) => { e.Cancel = true; cts.Cancel(); };

return await LegacyLoginHostRunner.RunAsync(
    new LegacyLoginHostRunOptions(LegacyLoginHostListenMode.RespectEnvironment),
    cts.Token).ConfigureAwait(false);
