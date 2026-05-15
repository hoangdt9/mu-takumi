// M5: Connect Server only (F4 06/03). Pair with Takumi.Server.LoginHost on TAKUMI_LOGIN_PORT.
using Takumi.Server.Connect;

using var cts = new CancellationTokenSource();
Console.CancelKeyPress += (_, e) => { e.Cancel = true; cts.Cancel(); };

return await ConnectServerHostRunner.RunAsync(cts.Token).ConfigureAwait(false);
