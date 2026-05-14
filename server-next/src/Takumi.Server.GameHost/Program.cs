using System.Globalization;
using System.Net.Sockets;
using Takumi.Server.Game;
using Takumi.Server.Game.Networking;
using Takumi.Server.Shared;

var port = int.TryParse(Environment.GetEnvironmentVariable("TAKUMI_GAME_PORT"), NumberStyles.Integer, CultureInfo.InvariantCulture, out var p)
    ? p
    : ListenPortFallbacks.GameTcp;

var joinVersion = JoinVersionEnv.ResolveFromEnvironment();
if (joinVersion.Length != 5)
{
    Console.Error.WriteLine("Join version must be exactly 5 bytes (wire form = MainInfo ClientVersion mapping).");
    return 1;
}

var reuseSocketAddr = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_REUSE_ADDR"), "1", StringComparison.OrdinalIgnoreCase);
var verbose = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_VERBOSE"), "1", StringComparison.OrdinalIgnoreCase)
                || string.Equals(Environment.GetEnvironmentVariable("DOTNET_ENVIRONMENT"), "Development", StringComparison.OrdinalIgnoreCase);

var (serverDecryptKeys, decryptKeysTag) = DecryptKeysLoader.LoadFromDec2OrDefault();

var keepAlive = GameKeepAliveEnv.ParseInterval();

using var cts = new CancellationTokenSource();
Console.CancelKeyPress += (_, e) =>
{
    e.Cancel = true;
    cts.Cancel();
};

Console.WriteLine(
    "Takumi.Server.GameHost pid={0} TAKUMI_GAME_PORT={1} joinVersion={2} decrypt={3} TAKUMI_REUSE_ADDR={4} verbose={5}",
    Environment.ProcessId,
    port,
    Convert.ToHexString(joinVersion),
    decryptKeysTag,
    reuseSocketAddr,
    verbose);

var options = new GameHostOptions
{
    Port = port,
    JoinVersion5 = joinVersion,
    Verbose = verbose,
    ReuseAddress = reuseSocketAddr,
    KeepAliveInterval = keepAlive,
};

try
{
    await TakumiGameHost.RunAsync(options, serverDecryptKeys, cts.Token).ConfigureAwait(false);
}
catch (SocketException ex) when (ex.SocketErrorCode == SocketError.AddressAlreadyInUse)
{
    return 1;
}

return 0;
