// Dedicated game TCP listener (M6): parity with Source/4.GameServer GCConnectClientSend first packet (C1 F1 00).
// When TAKUMI_ACCOUNTS + TAKUMI_SERVER_SERIAL are set, handles F1 01 login + character list + join map (same JSON roster as LegacyLoginHost).

using System.Globalization;
using System.Net.Sockets;
using Takumi.Server.Game;
using Takumi.Server.Hosting;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

RepoEnvLoader.ApplyDefaultsAndLocalEnv();
TakumiPostgresMirror.InitIfEnabled();
TakumiPostgresMirror.InitSessionHandoffIfEnabled();
TakumiPostgresMirror.InitMonsterSpawnIfEnabled();

if (!int.TryParse(
        Environment.GetEnvironmentVariable("TAKUMI_GAME_PORT"),
        NumberStyles.Integer,
        CultureInfo.InvariantCulture,
        out var gamePort)
    || gamePort is <= 0 or > 65535)
{
    Console.Error.WriteLine(
        "Takumi.Server.GameHost requires TAKUMI_GAME_PORT (1..65535). Set in server-next/.env — must match the port advertised in ConnectServer C1 F4 03.");
    return 1;
}

var joinVersion = GameWireEnv.ParseJoinVersion5(Environment.GetEnvironmentVariable("TAKUMI_JOIN_VERSION_HEX"))
                  ?? GameWireEnv.ParseJoinVersion5(Environment.GetEnvironmentVariable("TAKUMI_JOIN_VERSION"));
if (joinVersion is null || joinVersion.Length != 5)
{
    Console.Error.WriteLine(
        "Join version must be exactly 5 bytes (wire). Set TAKUMI_JOIN_VERSION=10405 or TAKUMI_JOIN_VERSION_HEX in env.defaults or .env.");
    return 1;
}

var joinIndex = 0;
if (ushort.TryParse(
        Environment.GetEnvironmentVariable("TAKUMI_GAME_JOIN_WIRE_INDEX"),
        NumberStyles.Integer,
        CultureInfo.InvariantCulture,
        out var ji))
{
    joinIndex = ji;
}

var (serverDecryptKeys, decryptKeysTag) = Season6ClientToServerDecryptSession.LoadServerDecryptKeysFromDec2OrEnv();

var verbose = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_VERBOSE"), "1", StringComparison.OrdinalIgnoreCase)
              || string.Equals(Environment.GetEnvironmentVariable("DOTNET_ENVIRONMENT"), "Development", StringComparison.OrdinalIgnoreCase);

var reuseSocketAddr = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_REUSE_ADDR"), "1", StringComparison.OrdinalIgnoreCase);

var accounts = GameWireEnv.ParseAccounts(Environment.GetEnvironmentVariable("TAKUMI_ACCOUNTS"));
var serverSerial = GameWireEnv.ParseSerial16(Environment.GetEnvironmentVariable("TAKUMI_SERVER_SERIAL"));
if (accounts is { Count: > 0 } && (serverSerial is null || serverSerial.Length != 16))
{
    Console.Error.WriteLine(
        "TAKUMI_ACCOUNTS is set but TAKUMI_SERVER_SERIAL must be 16 ASCII bytes (same as LegacyLoginHost / client Data).");
    return 1;
}

var skipAutoCharList = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_SKIP_AUTO_CHARLIST"), "1", StringComparison.OrdinalIgnoreCase);

var requireLoginHandoff = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF"), "1", StringComparison.OrdinalIgnoreCase)
                        || string.Equals(Environment.GetEnvironmentVariable("TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF"), "true", StringComparison.OrdinalIgnoreCase);
var handoffMatchIp = !string.Equals(Environment.GetEnvironmentVariable("TAKUMI_GAME_HANDOFF_MATCH_IP"), "0", StringComparison.OrdinalIgnoreCase);

var requireSignedSessionTicketWire = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_GAME_TICKET_WIRE"), "1", StringComparison.OrdinalIgnoreCase)
                                   || string.Equals(Environment.GetEnvironmentVariable("TAKUMI_GAME_TICKET_WIRE"), "true", StringComparison.OrdinalIgnoreCase);

var protectWireRaw = Environment.GetEnvironmentVariable("TAKUMI_GAME_CLIENT_PROTECT_WIRE");
var protectCustomer = Environment.GetEnvironmentVariable("TAKUMI_PROTECT_CUSTOMER_NAME");
var protectOutbound = GameWireEnv.ResolveClientProtectOutboundKeys(protectWireRaw, protectCustomer, serverSerial);

// Android (ENCRYPT_STATE): CheckSocketPort + DecryptData on recv for GS-range peer ports — plain C1 on wire becomes garbage (logcat: packet sync lost).
if (protectOutbound is null
    && gamePort is >= 55901 and <= 55999
    && !string.Equals(protectWireRaw?.Trim(), "0", StringComparison.OrdinalIgnoreCase)
    && !string.Equals(protectWireRaw?.Trim(), "false", StringComparison.OrdinalIgnoreCase))
{
    Console.Error.WriteLine(
        "[game-host] WARN: client protect wire is OFF on port {0} (typical Android GS range). " +
        "Unset TAKUMI_GAME_CLIENT_PROTECT_WIRE or set to 1; plain join will break Android recv (packet sync lost / intro hang).",
        gamePort);
}

if (requireSignedSessionTicketWire)
{
    if (TakumiPostgresMirror.SessionHandoff is null)
    {
        Console.Error.WriteLine(
            "TAKUMI_GAME_TICKET_WIRE=1 requires Postgres session handoff (set TAKUMI_SESSION_HANDOFF_DB=1 and a valid PG connection string).");
        return 1;
    }

    var wireKey = SessionTicketSignature602.ResolveHmacKeyFromEnv();
    if (wireKey.Length < 8)
    {
        Console.Error.WriteLine(
            "TAKUMI_GAME_TICKET_WIRE=1 requires TAKUMI_SESSION_TICKET_HMAC_KEY (UTF-8, at least 8 bytes), shared with LegacyLoginHost.");
        return 1;
    }
}

var options = new GamePortListenOptions
{
    ServerDecryptKeys = serverDecryptKeys,
    JoinVersion5 = joinVersion,
    JoinWireIndex = (ushort)joinIndex,
    Verbose = verbose,
    ReuseAddress = reuseSocketAddr,
    AuthAccounts = accounts,
    AuthServerSerial16 = serverSerial,
    SkipAutoCharacterList = skipAutoCharList,
    RequireLoginPostgresHandoff = requireLoginHandoff,
    LoginHandoffMatchClientIp = handoffMatchIp,
    RequireSignedSessionTicketWire = requireSignedSessionTicketWire,
    ClientProtectOutboundKeys = protectOutbound,
};

using var cts = new CancellationTokenSource();
Console.CancelKeyPress += (_, e) => { e.Cancel = true; cts.Cancel(); };

Console.WriteLine(
    "Takumi.Server.GameHost\n" +
    "  listen:        *:{0}\n" +
    "  join wire idx: {1}\n" +
    "  join version:  {2}\n" +
    "  decrypt keys:  {3}\n" +
    "  accounts:      {4}\n" +
    "  verbose RX:    {5}\n" +
    "  handoff DB:    {6} (match IP: {7})\n" +
    "  ticket wire:   {8}\n" +
    "  client protect wire (Android GS port): {9}\n" +
    "Ctrl+C to stop.",
    gamePort,
    joinIndex,
    Convert.ToHexString(joinVersion),
    decryptKeysTag,
    accounts is null ? "(none — bootstrap-only)" : string.Join(", ", accounts.Keys) + ":***",
    verbose,
    requireLoginHandoff,
    handoffMatchIp,
    requireSignedSessionTicketWire,
    protectOutbound is null ? "off" : "on");

try
{
    await GameListenHost.RunAsync(gamePort, options, cts.Token).ConfigureAwait(false);
}
catch (SocketException ex) when (ex.SocketErrorCode == SocketError.AddressAlreadyInUse)
{
    Console.Error.WriteLine(
        "Cannot bind game port {0}: address already in use. Stop LegacyLoginHost on same port or pick another TAKUMI_GAME_PORT.\n" +
        "  Check: lsof -nP -iTCP:{0} -sTCP:LISTEN",
        gamePort);
    return 1;
}

return 0;
