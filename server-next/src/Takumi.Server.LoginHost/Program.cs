// M5: login/game TCP only (no Connect on TAKUMI_CONNECT_PORT). Pair with Takumi.Server.ConnectHost.
using Takumi.Server.Hosting;
using Takumi.Server.LegacyLoginHost;

RepoEnvLoader.ApplyDefaultsAndLocalEnv();

using var cts = new CancellationTokenSource();
Console.CancelKeyPress += (_, e) => { e.Cancel = true; cts.Cancel(); };

return await LegacyLoginHostRunner.RunAsync(
    new LegacyLoginHostRunOptions(LegacyLoginHostListenMode.LoginTcpOnly),
    cts.Token).ConfigureAwait(false);
