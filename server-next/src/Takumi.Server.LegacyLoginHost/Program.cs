// Minimal Takumi / Season6-style login listener for Android JNI path:
// - Send plain C1 F1 00 (join) immediately after TCP accept (same as GameServer GCConnectClientSend).
// - Decrypt client->server with OpenMU PipelinedDecryptor (SimpleModulus then Xor32), matching Android SendGameEncrypted.
//   IMPORTANT: keys must match the client's Data/Dec2.dat (set TAKUMI_DEC2_PATH or place Data/Dec2.dat next to the exe).
// - Parse C3/C1 F1 01 login, validate version + serial + credentials, answer C1 F1 01, then C1 F3 00 (character list).
// - Optional roster persistence per account: TAKUMI_ROSTER_DIR or ./takumi-roster (cwd) so reconnect shows saved characters.

using System.Buffers;
using System.Buffers.Binary;
using System.Globalization;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using System.Threading;
using Microsoft.Extensions.Logging.Abstractions;
using MUnique.OpenMU.Network;
using MUnique.OpenMU.Network.SimpleModulus;
using MUnique.OpenMU.Network.Xor;
using MUnique.OpenMU.PlugIns;
using Pipelines.Sockets.Unofficial;

const int DefaultPort = 44606;

// Wire bytes for ServerVersion = "1.04.05" using GameServerInfo indices [0],[2],[3],[5],[6] -> ASCII "10405".
var defaultJoinVersion = Encoding.ASCII.GetBytes("10405");
// GameServerInfo - Common.ini default in this repo (must match client Main / APK serial).
var defaultServerSerial = Encoding.ASCII.GetBytes("TbYehR2hFUPBKgZj");

var port = int.TryParse(Environment.GetEnvironmentVariable("TAKUMI_LOGIN_PORT"), out var p) ? p : DefaultPort;
var joinVersion = ParseHexOrAscii5(Environment.GetEnvironmentVariable("TAKUMI_JOIN_VERSION_HEX"))
                  ?? ParseHexOrAscii5(Environment.GetEnvironmentVariable("TAKUMI_JOIN_VERSION"))
                  ?? defaultJoinVersion;
if (joinVersion.Length != 5)
{
    Console.Error.WriteLine("Join version must be exactly 5 bytes (wire form = MainInfo ClientVersion mapping / server m_ServerVersion).");
    return 1;
}

var serverSerial = ParseSerial(Environment.GetEnvironmentVariable("TAKUMI_SERVER_SERIAL")) ?? defaultServerSerial;
if (serverSerial.Length != 16)
{
    Console.Error.WriteLine("Server serial must be 16 bytes ASCII.");
    return 1;
}

var accounts = ParseAccounts(Environment.GetEnvironmentVariable("TAKUMI_ACCOUNTS"))
               ?? new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
               {
                   ["test"] = "test",
                   ["admin"] = "admin",
               };

var (serverDecryptKeys, decryptKeysTag) = LoadDecryptKeysFromDec2OrDefault();

var verbose = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_VERBOSE"), "1", StringComparison.OrdinalIgnoreCase)
              || string.Equals(Environment.GetEnvironmentVariable("DOTNET_ENVIRONMENT"), "Development", StringComparison.OrdinalIgnoreCase);

// Minimal Connect Server (F4 06 / F4 03) for Android LAN QA. Set TAKUMI_CONNECT_PORT=0 to disable.
var connectPort = 44605;
if (!string.IsNullOrWhiteSpace(Environment.GetEnvironmentVariable("TAKUMI_CONNECT_PORT"))
    && int.TryParse(Environment.GetEnvironmentVariable("TAKUMI_CONNECT_PORT"), out var cpEnv))
{
    connectPort = cpEnv;
}

var publicHost = Environment.GetEnvironmentVariable("TAKUMI_PUBLIC_HOST")?.Trim();
if (string.IsNullOrEmpty(publicHost))
{
    Console.Error.WriteLine(
        "[connect] WARNING: TAKUMI_PUBLIC_HOST unset — using 127.0.0.1. Phones on Wi‑Fi cannot reach that; set server-next/.env (LAN IP).");
    publicHost = "127.0.0.1";
}

// Connect-server list (C2 F4 06): Takumi maps connect index → ServerList.bmd group via (index/20). If your
// Data/Local/ServerList.bmd has no group 0, the server-select UI stays empty — set TAKUMI_CS_CONNECT_BASE to 20, 40, …
var csConnectBase = 0;
if (!string.IsNullOrWhiteSpace(Environment.GetEnvironmentVariable("TAKUMI_CS_CONNECT_BASE"))
    && int.TryParse(Environment.GetEnvironmentVariable("TAKUMI_CS_CONNECT_BASE"), NumberStyles.Integer, CultureInfo.InvariantCulture, out var csb)
    && csb is >= 0 and <= 65532)
{
    csConnectBase = csb;
}

var csConnectCount = 3;
if (!string.IsNullOrWhiteSpace(Environment.GetEnvironmentVariable("TAKUMI_CS_CONNECT_COUNT"))
    && int.TryParse(Environment.GetEnvironmentVariable("TAKUMI_CS_CONNECT_COUNT"), NumberStyles.Integer, CultureInfo.InvariantCulture, out var csc)
    && csc is >= 1 and <= 32)
{
    csConnectCount = csc;
}

var connectServerListPacket = ConnectWire.BuildServerList602(csConnectBase, csConnectCount, loadPercent: 0x0A);

// SO_REUSEADDR can allow a *second* listener on the same port while an older host is still running (e.g. stray dotnet),
// so phones may hit the old process and keep seeing C1 4A (33-byte slots). Default: strict bind (reuse off).
var reuseSocketAddr = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_REUSE_ADDR"), "1", StringComparison.OrdinalIgnoreCase);

using var cts = new CancellationTokenSource();
Console.CancelKeyPress += (_, e) => { e.Cancel = true; cts.Cancel(); };

TcpListener listener;
try
{
    listener = new TcpListener(IPAddress.Any, port);
    listener.Server.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, reuseSocketAddr);
    listener.Start();
    Console.Error.WriteLine(
        "[boot] LegacyLoginHost pid={0} gamePort={1} charlist slot bytes=34 (2 chars => wire C1 4C / 76 bytes). asm={2}\n" +
        "[boot] SO_REUSEADDR={3} (set TAKUMI_REUSE_ADDR=1 only if you hit TIME_WAIT bind failures after quick restarts)\n" +
        "[boot] If logcat still shows C1 4A / nbytes=74, another process may share this port — check: lsof -nP -iTCP:{1} -sTCP:LISTEN",
        Environment.ProcessId,
        port,
        System.Reflection.Assembly.GetExecutingAssembly().Location,
        reuseSocketAddr);
}
catch (SocketException ex) when (ex.SocketErrorCode == SocketError.AddressAlreadyInUse)
{
    Console.Error.WriteLine(
        "Cannot bind port {0}: address already in use. Stop the other listener (docker/old dotnet) or use another port.\n" +
        "  Check: lsof -nP -iTCP:{0} -sTCP:LISTEN\n" +
        "  Then: kill <PID>\n" +
        "  Or:   TAKUMI_LOGIN_PORT=44607 dotnet run ...",
        port);
    return 1;
}

if (connectPort > 0)
{
    _ = Task.Run(
        async () =>
        {
            try
            {
                await RunMinimalConnectServerAsync(
                        connectPort,
                        publicHost!,
                        (ushort)port,
                        verbose,
                        connectServerListPacket,
                        cts.Token)
                    .ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cts.IsCancellationRequested)
            {
                // shutdown
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("[connect] fatal: {0}", ex);
            }
        },
        cts.Token);
}

Console.WriteLine(
    "Takumi.Server.LegacyLoginHost listening on *:{0}. Ctrl+C to stop.\n" +
    "  Join version (5 bytes wire): {1}\n" +
    "  Server serial (16 bytes):    {2}\n" +
    "  Accounts:                    {3}\n" +
    "  SimpleModulus (server decrypt): {4}",
    port,
    Convert.ToHexString(joinVersion),
    Encoding.ASCII.GetString(serverSerial),
    string.Join(", ", accounts.Select(kv => kv.Key + ":***")),
    decryptKeysTag);

if (connectPort > 0)
{
    Console.WriteLine(
        "Minimal Connect Server on *:{0} -> TAKUMI_PUBLIC_HOST={1} game/login port={2} (verbose={3})\n" +
        "  F4 06 list: connectBase={4} count={5} (set TAKUMI_CS_CONNECT_BASE to match Data/Local/ServerList.bmd group index×20)",
        connectPort,
        publicHost,
        port,
        verbose,
        csConnectBase,
        csConnectCount);
}

try
{
    while (!cts.IsCancellationRequested)
    {
        TcpClient tcp;
        try
        {
            tcp = await listener.AcceptTcpClientAsync(cts.Token).ConfigureAwait(false);
        }
        catch (OperationCanceledException) when (cts.IsCancellationRequested)
        {
            break;
        }

        var acceptedFrom = tcp.Client.RemoteEndPoint?.ToString() ?? "?";
        var acceptMsg = $"[game] TCP accept from {acceptedFrom} (game/login port {port}) — next: join F1 00";
        Console.WriteLine(acceptMsg);
        Console.Error.WriteLine(acceptMsg);

        _ = HandleClientAsync(tcp, joinVersion, serverSerial, accounts, serverDecryptKeys, cts.Token);
    }
}
finally
{
    listener.Stop();
}

return 0;

/// <summary>
/// Inverse of StreamPacketEngine <c>XorData</c> / <c>mu::Xor32Encrypt</c> (same XOR formula as SimpleModulusCrypt.h).
/// Must run <b>high index → low</b> like <c>Xor32Decrypt</c>: forward in-place decode would replace E[i-1] with plaintext
/// and break the chain for byte i (see android/SimpleModulusCrypt.h).
/// </summary>
static void DecodeTakumiStreamXor(Span<byte> buffer, int firstXorIndex)
{
    ReadOnlySpan<byte> filter =
    [
        0xAB, 0x11, 0xCD, 0xFE, 0x18, 0x23, 0xC5, 0xA3,
        0xCA, 0x33, 0xC1, 0xCC, 0x66, 0x67, 0x21, 0xF3,
        0x32, 0x12, 0x15, 0x35, 0x29, 0xFF, 0xFE, 0x1D,
        0x44, 0xEF, 0xCD, 0x41, 0x26, 0x3C, 0x4E, 0x4D,
    ];
    for (var i = buffer.Length - 1; i >= firstXorIndex; i--)
    {
        buffer[i] ^= (byte)(buffer[i - 1] ^ filter[i % 32]);
    }
}

static async Task HandleClientAsync(
    TcpClient tcp,
    byte[] joinVersion,
    byte[] serverSerial,
    IReadOnlyDictionary<string, string> accounts,
    SimpleModulusKeys serverDecryptKeys,
    CancellationToken ct)
{
    var remote = tcp.Client.RemoteEndPoint?.ToString() ?? "?";
    try
    {
        tcp.NoDelay = true;
        var socket = tcp.Client;
        var socketConnection = SocketConnection.Create(socket);
        using var connection = new Connection(
            socketConnection,
            new PipelinedDecryptor(socketConnection.Input, serverDecryptKeys, DefaultKeys.Xor32Key),
            encryptionPipe: null,
            new NullLogger<Connection>());

        var join = BuildJoinPacket(result: 1, index: 0, joinVersion);
        await connection.Output.WriteAsync(join, ct).ConfigureAwait(false);
        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
        // Stderr too — easier to spot when stdout is buffered or mixed with other hosts.
        var joinMsg = $"[{remote}] sent join C1 F1 00 ({join.Length} bytes) — client should log recv tcp first byte C1";
        Console.WriteLine(joinMsg);
        Console.Error.WriteLine(joinMsg);

        var loggedIn = false;
        string? loggedAccountId = null;
        var roster = new List<CharacterRosterEntry>();
        // PacketReceived may invoke handlers concurrently before the previous async handler awaits;
        // a follow-up 12-byte F3 request could run before loggedIn=true → list ignored. Serialize per connection.
        using var packetGate = new SemaphoreSlim(1, 1);

        connection.PacketReceived += OnPacketAsync;

        await connection.BeginReceiveAsync().ConfigureAwait(false);

        async ValueTask OnPacketAsync(ReadOnlySequence<byte> packetSeq)
        {
            await packetGate.WaitAsync(ct).ConfigureAwait(false);
            try
            {
            var packet = packetSeq.ToArray();
            if (packet.Length == 0)
            {
                return;
            }

            var t = packet[0];
            Console.WriteLine("[{0}] decrypted len={1} head=0x{2:X2}", remote, packet.Length, t);

            if (loggedIn && packet.Length == 15 && packet[0] == 0xC1)
            {
                Console.WriteLine(
                    "[{0}] trace C1×15 b1=0x{1:X2} b2=0x{2:X2} b3=0x{3:X2} hex={4}",
                    remote,
                    packet[1],
                    packet[2],
                    packet[3],
                    Convert.ToHexString(packet));
            }

            // Create character F3 01: Android SendCreateCharacter (C1 0F F3 01 + name[10] + class); Takumi expects
            // PRECEIVE_CREATE_CHARACTER (19 bytes) to close the wait dialog — see WSclient ReceiveCreateCharacter.
            if (TryFindCreateCharacterRequest(packet, remote, out var createOff, out var nameCopy, out var reqClass))
            {
                if (!loggedIn)
                {
                    Console.WriteLine("[{0}] create character (F3 01) before login — ignored", remote);
                    return;
                }

                UpsertRoster(roster, nameCopy, reqClass, level: 1);
                var slotByte = (byte)(roster.Count - 1);
                var resp = BuildCreateCharacterSuccessPacket(nameCopy, slotByte, level: 1, serverClass: reqClass);
                await connection.Output.WriteAsync(resp, ct).ConfigureAwait(false);
                await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                if (!string.IsNullOrEmpty(loggedAccountId))
                {
                    SavePersistedRoster(loggedAccountId, roster);
                }

                Console.WriteLine(
                    "[{0}] sent create character OK (F3 01) slot={1} class=0x{2:X2} name='{3}' frame@{4}",
                    remote,
                    slotByte,
                    reqClass,
                    Encoding.ASCII.GetString(nameCopy).TrimEnd('\0'),
                    createOff);
                return;
            }

            // Join map: desktop SendRequestJoinMapServer (F3 03) or Android SendSelectCharacter (F3 15) then ReceiveJoinMapServer.
            if (TryFindCharacterJoinRequest(packet, out var joinOff, out var joinName10))
            {
                if (!loggedIn)
                {
                    Console.WriteLine("[{0}] join map (F3 03/15) before login — ignored", remote);
                    return;
                }

                var picked = FindRosterEntry(roster, joinName10);
                if (picked is null)
                {
                    Console.WriteLine(
                        "[{0}] join map ignored — no roster match for name='{1}' (roster={2}) hex={3}",
                        remote,
                        Encoding.ASCII.GetString(joinName10).TrimEnd('\0'),
                        roster.Count,
                        Convert.ToHexString(packet));
                    return;
                }

                var joinPkt = BuildJoinMapServer602(picked);
                await connection.Output.WriteAsync(joinPkt, ct).ConfigureAwait(false);
                await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                Console.WriteLine(
                    "[{0}] sent join map (F3 03) map={1} xy=({2},{3}) name='{4}' rosterClass=0x{5:X2} frame@{6}",
                    remote,
                    joinPkt[6],
                    joinPkt[4],
                    joinPkt[5],
                    Encoding.ASCII.GetString(picked.Name10).TrimEnd('\0'),
                    picked.ServerClass,
                    joinOff);
                return;
            }

            if (loggedIn
                && string.Equals(Environment.GetEnvironmentVariable("TAKUMI_VERBOSE"), "1", StringComparison.OrdinalIgnoreCase)
                && packet.Length == 15
                && packet[0] == 0xC1
                && packet[2] == 0xF3)
            {
                Span<byte> diag = stackalloc byte[15];
                packet.CopyTo(diag);
                Console.WriteLine(
                    "[{0}] F3 create-frame NOT matched — wireSub@3=0x{1:X2} hex={2}",
                    remote,
                    diag[3],
                    Convert.ToHexString(packet));
                for (var z = 0; z < 2 && diag[3] != 0x01; z++)
                {
                    DecodeTakumiStreamXor(diag, 3);
                    Console.WriteLine("[{0}] F3 create-frame diag pass {1} sub@3=0x{2:X2}", remote, z + 1, diag[3]);
                }
            }

            // Character list request F3 00: Android `SendRequestCharacterList` → plain C1 F3 00 + language, XOR on wire.
            // After PipelinedDecryptor the MU frame may not start at index 0 (prefix / coalescing); scan for C1|C3 + F3 00.
            // Ref: takumi Source/5.Main/source/android/AndroidNetwork.cpp SendRequestCharacterList + SendGameEncrypted.
            var listReq = TryFindCharacterListRequest(packet, out var listFrameOffset);
            if (!listReq
                && loggedIn
                && packet.Length == 12
                && packet[0] == 0xC3)
            {
                // Last resort: Takumi+OpenMU pipeline often yields 12-byte C3 frames where Head/Sub follow a serial byte (not at +2/+3).
                Console.WriteLine(
                    "[{0}] treating len=12 C3 as character-list req (hex={1})",
                    remote,
                    Convert.ToHexString(packet));
                listReq = true;
                listFrameOffset = 0;
            }

            if (listReq)
            {
                if (!loggedIn)
                {
                    Console.WriteLine("[{0}] F3 00 before login — ignored", remote);
                    return;
                }

                var list = roster.Count > 0 ? BuildCharacterList602(roster) : BuildEmptyCharacterList602();
                LogCharacterListWire(remote, list, "on F3 00 request");
                await connection.Output.WriteAsync(list, ct).ConfigureAwait(false);
                await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                if (roster.Count > 0)
                {
                    var entryBytes = (list.Length - 8) / roster.Count;
                    var listLog =
                        $"[{remote}] sent character list F3 00 totalLen={list.Length} count={roster.Count} entryWireBytes={entryBytes} (expect 34 w/ padding; adb C1 size 0x{list[1]:X2}) frameOffset={listFrameOffset} frameHead=0x{packet[listFrameOffset]:X2}";
                    Console.WriteLine(listLog);
                    Console.Error.WriteLine(listLog);
                }
                else
                {
                    Console.WriteLine(
                        "[{0}] sent character list F3 00 (empty) totalLen={1} frameOffset={2} frameHead=0x{3:X2}",
                        remote,
                        list.Length,
                        listFrameOffset,
                        packet[listFrameOffset]);
                }

                return;
            }

            if (loggedIn && packet.Length <= 24)
            {
                Console.WriteLine("[{0}] post-login unmatched hex={1}", remote, Convert.ToHexString(packet));
            }

            // Login: Android builds inner packet as C3 F1 01 ... (see AndroidNetwork.cpp SendGameLogin).
            byte head;
            byte sub;
            int payloadOffset;
            if (t == 0xC3 && packet.Length >= 4)
            {
                head = packet[2];
                sub = packet[3];
                payloadOffset = 4;
            }
            else if (t == 0xC1 && packet.Length >= 4)
            {
                head = packet[2];
                sub = packet[3];
                payloadOffset = 4;
            }
            else
            {
                return;
            }

            // Android plain layout: ... account[10] password[20] tick[4] version[5] serial[16] (59 bytes after C3 header).
            if (head != 0xF1 || sub != 0x01 || packet.Length < 59)
            {
                if (loggedIn
                    && packet.Length <= 48
                    && string.Equals(Environment.GetEnvironmentVariable("TAKUMI_VERBOSE"), "1", StringComparison.OrdinalIgnoreCase))
                {
                    Console.WriteLine(
                        "[{0}] not login pkt — hex={1} head@2=0x{2:X2} sub@3=0x{3:X2}",
                        remote,
                        Convert.ToHexString(packet),
                        head,
                        sub);
                }

                return;
            }

            // Match takumi GameServer Protocol.cpp CGConnectAccountRecv: PacketArgumentDecrypt(account, …, 10)
            // and PacketArgumentDecrypt(password, …, 10) — only the first 10 password bytes are auth-relevant.
            var idEnc = packet.AsSpan(payloadOffset, 10).ToArray();
            var passEnc = packet.AsSpan(payloadOffset + 10, 10).ToArray();
            BuxXor(idEnc);
            BuxXor(passEnc);
            var id = Encoding.ASCII.GetString(idEnc).TrimEnd('\0', ' ');
            var pass = Encoding.ASCII.GetString(passEnc).TrimEnd('\0', ' ');

            var clientVer = packet.AsSpan(payloadOffset + 34, 5);
            var clientSer = packet.AsSpan(payloadOffset + 39, 16);

            if (!clientVer.SequenceEqual(joinVersion))
            {
                Console.WriteLine("[{0}] login rejected: client version {1} != join {2}", remote, Convert.ToHexString(clientVer), Convert.ToHexString(joinVersion));
                await WriteLoginResultAsync(connection, 0x06, ct).ConfigureAwait(false);
                return;
            }

            if (!clientSer.SequenceEqual(serverSerial))
            {
                Console.WriteLine("[{0}] login rejected: serial mismatch", remote);
                await WriteLoginResultAsync(connection, 0x06, ct).ConfigureAwait(false);
                return;
            }

            if (!accounts.TryGetValue(id, out var okPass) || okPass != pass)
            {
                Console.WriteLine("[{0}] login rejected: bad id/password for '{1}'", remote, id);
                await WriteLoginResultAsync(connection, 0x00, ct).ConfigureAwait(false);
                return;
            }

            loggedIn = true;
            loggedAccountId = id;
            roster.Clear();
            LoadPersistedRoster(id, roster);
            Console.WriteLine(
                "[{0}] login ok id={1} rosterPersisted={2} rosterCount={3}",
                remote,
                id,
                GetRosterFilePath(id),
                roster.Count);
            // 0x01 and 0x20 are both treated as success in Takumi WSclient TranslateProtocol.
            await WriteLoginResultAsync(connection, 0x01, ct).ConfigureAwait(false);

            // Scene switches to character select and client sends F3 00; push list from disk (or empty) (disable with TAKUMI_SKIP_AUTO_CHARLIST=1).
            if (!string.Equals(Environment.GetEnvironmentVariable("TAKUMI_SKIP_AUTO_CHARLIST"), "1", StringComparison.Ordinal))
            {
                var list = roster.Count > 0 ? BuildCharacterList602(roster) : BuildEmptyCharacterList602();
                LogCharacterListWire(remote, list, "after login (auto)");
                await connection.Output.WriteAsync(list, ct).ConfigureAwait(false);
                await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                if (roster.Count > 0)
                {
                    var entryBytes = (list.Length - 8) / roster.Count;
                    var listLogAfter =
                        $"[{remote}] sent character list F3 00 after login totalLen={list.Length} count={roster.Count} entryWireBytes={entryBytes} (expect 34; C1 size 0x{list[1]:X2})";
                    Console.WriteLine(listLogAfter);
                    Console.Error.WriteLine(listLogAfter);
                }
                else
                {
                    Console.WriteLine("[{0}] sent empty character list F3 00 after login totalLen={1}", remote, list.Length);
                }
            }
            }
            finally
            {
                packetGate.Release();
            }
        }
    }
    catch (OperationCanceledException) when (ct.IsCancellationRequested)
    {
        // shutdown
    }
    catch (Exception ex)
    {
        Console.WriteLine("[{0}] error: {1}", remote, ex.Message);
    }
    finally
    {
        tcp.Dispose();
    }
}

static byte[] BuildServerInfoPacket(string ip, ushort gamePort)
{
    var pkt = new byte[22];
    pkt[0] = 0xC1;
    pkt[1] = 22;
    pkt[2] = 0xF4;
    pkt[3] = 0x03;
    var ipBytes = Encoding.ASCII.GetBytes(ip);
    if (ipBytes.Length > 16)
    {
        throw new ArgumentException("TAKUMI_PUBLIC_HOST must be at most 16 ASCII characters for ServerInfo packet.", nameof(ip));
    }

    ipBytes.CopyTo(pkt.AsSpan(4));
    BinaryPrimitives.WriteUInt16LittleEndian(pkt.AsSpan(20, 2), gamePort);
    return pkt;
}

static async Task RunMinimalConnectServerAsync(
    int listenPort,
    string publicHost,
    ushort gamePort,
    bool verbose,
    byte[] serverList602,
    CancellationToken ct)
{
    var listener = new TcpListener(IPAddress.Any, listenPort);
    var reuse = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_REUSE_ADDR"), "1", StringComparison.OrdinalIgnoreCase);
    listener.Server.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, reuse);
    listener.Start();
    Console.WriteLine("[connect] listening on *:{0}", listenPort);
    try
    {
        while (!ct.IsCancellationRequested)
        {
            var tcp = await listener.AcceptTcpClientAsync(ct).ConfigureAwait(false);
            _ = HandleMinimalConnectClientAsync(tcp, publicHost, gamePort, verbose, serverList602, ct);
        }
    }
    catch (OperationCanceledException) when (ct.IsCancellationRequested)
    {
        // shutdown
    }
    finally
    {
        listener.Stop();
    }
}

static async Task HandleMinimalConnectClientAsync(
    TcpClient tcp,
    string publicHost,
    ushort gamePort,
    bool verbose,
    byte[] serverList602,
    CancellationToken ct)
{
    var remote = tcp.Client.RemoteEndPoint?.ToString() ?? "?";
    var acceptMsg = $"[connect] TCP accept from {remote} (Connect Server — waiting for F4 06 / F4 03)";
    Console.WriteLine(acceptMsg);
    Console.Error.WriteLine(acceptMsg);
    try
    {
        tcp.NoDelay = true;
        await using var stream = tcp.GetStream();
        var buf = new byte[512];
        while (!ct.IsCancellationRequested)
        {
            var n = await stream.ReadAsync(buf.AsMemory(0, buf.Length), ct).ConfigureAwait(false);
            if (n <= 0)
            {
                break;
            }

            var hex = Convert.ToHexString(buf.AsSpan(0, n));
            Console.WriteLine("[connect] recv {0}: {1}", remote, hex);

            if (n >= 4 && buf[0] == 0xC1 && buf[2] == 0xF4 && buf[3] == 0x06)
            {
                await stream.WriteAsync(serverList602, ct).ConfigureAwait(false);
                await stream.FlushAsync(ct).ConfigureAwait(false);
                Console.WriteLine("[connect] sent {0}: ServerList ({1} bytes)", remote, serverList602.Length);
            }
            else if (n >= 4 && buf[0] == 0xC1 && buf[2] == 0xF4 && buf[3] == 0x03)
            {
                var pkt = BuildServerInfoPacket(publicHost, gamePort);
                await stream.WriteAsync(pkt, ct).ConfigureAwait(false);
                await stream.FlushAsync(ct).ConfigureAwait(false);
                Console.WriteLine(
                    "[connect] sent {0}: ServerInfo ip={1} port={2} ({3} bytes)",
                    remote,
                    publicHost,
                    gamePort,
                    pkt.Length);
            }
            else if (verbose)
            {
                Console.WriteLine("[connect] ignored packet from {0} (see hex above)", remote);
            }
        }
    }
    catch (OperationCanceledException) when (ct.IsCancellationRequested)
    {
        // shutdown
    }
    catch (Exception ex)
    {
        Console.WriteLine("[connect] error {0}: {1}", remote, ex.Message);
    }
}

static byte[] BuildJoinPacket(byte result, ushort index, ReadOnlySpan<byte> version5)
{
    var p = new byte[12];
    p[0] = 0xC1;
    p[1] = 12;
    p[2] = 0xF1;
    p[3] = 0x00;
    p[4] = result;
    p[5] = (byte)((index >> 8) & 0xFF);
    p[6] = (byte)(index & 0xFF);
    version5.CopyTo(p.AsSpan(7));
    return p;
}

static ReadOnlyMemory<byte> BuildLoginResultPacket(byte result)
{
    // PMSG_CONNECT_ACCOUNT_SEND: PSBMSG_HEAD (C1,size,F1,01) + result
    return new byte[] { 0xC1, 0x05, 0xF1, 0x01, result };
}

static byte[] BuildEmptyCharacterList602()
{
    // PMSG_CHARACTER_LIST_SEND with ExtWarehouse (Season 6+ layout, 8 bytes total).
    return new byte[] { 0xC1, 0x08, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00 };
}

static string GetRosterRoot()
{
    var env = Environment.GetEnvironmentVariable("TAKUMI_ROSTER_DIR")?.Trim();
    if (!string.IsNullOrEmpty(env))
    {
        return Path.GetFullPath(env);
    }

    return Path.GetFullPath(Path.Combine(Environment.CurrentDirectory, "takumi-roster"));
}

static string SanitizeAccountForFile(string accountId)
{
    Span<char> buf = stackalloc char[Math.Min(accountId.Length, 48)];
    var n = 0;
    foreach (var c in accountId)
    {
        if (char.IsAsciiLetterOrDigit(c) || c == '_')
        {
            if (n < buf.Length)
            {
                buf[n++] = c;
            }
        }
    }

    return n == 0 ? "account" : new string(buf[..n]);
}

static string GetRosterFilePath(string accountId) => Path.Combine(GetRosterRoot(), SanitizeAccountForFile(accountId) + ".json");

static void LoadPersistedRoster(string accountId, List<CharacterRosterEntry> roster)
{
    roster.Clear();
    var path = GetRosterFilePath(accountId);
    if (!File.Exists(path))
    {
        return;
    }

    try
    {
        string json;
        lock (RosterIo.Lock)
        {
            json = File.ReadAllText(path);
        }

        var root = JsonSerializer.Deserialize<RosterPersistRoot>(json, RosterIo.Json);
        if (root?.Characters is null)
        {
            return;
        }

        foreach (var c in root.Characters)
        {
            if (string.IsNullOrWhiteSpace(c.Name))
            {
                continue;
            }

            var nm = new byte[10];
            var enc = Encoding.ASCII.GetBytes(c.Name.Trim());
            Buffer.BlockCopy(enc, 0, nm, 0, Math.Min(10, enc.Length));
            roster.Add(new CharacterRosterEntry { Name10 = nm, ServerClass = c.ServerClass, Level = c.Level });
        }
    }
    catch (Exception ex)
    {
        Console.WriteLine("[roster] load failed {0}: {1}", path, ex.Message);
    }
}

static void SavePersistedRoster(string accountId, IReadOnlyList<CharacterRosterEntry> roster)
{
    var root = new RosterPersistRoot();
    foreach (var e in roster)
    {
        var name = Encoding.ASCII.GetString(e.Name10).TrimEnd('\0', ' ');
        if (string.IsNullOrEmpty(name))
        {
            continue;
        }

        root.Characters.Add(new RosterPersistChar { Name = name, ServerClass = e.ServerClass, Level = e.Level });
    }

    var path = GetRosterFilePath(accountId);
    try
    {
        var dir = Path.GetDirectoryName(path);
        if (!string.IsNullOrEmpty(dir))
        {
            Directory.CreateDirectory(dir);
        }

        var json = JsonSerializer.Serialize(root, RosterIo.Json);
        lock (RosterIo.Lock)
        {
            File.WriteAllText(path, json);
        }
    }
    catch (Exception ex)
    {
        Console.WriteLine("[roster] save failed {0}: {1}", path, ex.Message);
    }
}

/// <summary>Takumi PRECEIVE_CREATE_CHARACTER: PBMSG + SubCode + Result + ID[10] + Index + Level + Class (19 bytes).</summary>
static ReadOnlyMemory<byte> BuildCreateCharacterSuccessPacket(byte[] name10, byte slot, ushort level, byte serverClass)
{
    var p = new byte[19];
    p[0] = 0xC1;
    p[1] = 19;
    p[2] = 0xF3;
    p[3] = 0x01;
    p[4] = 1; // success — ReceiveCreateCharacter checks Result==1
    var dst = p.AsSpan(5, 10);
    dst.Clear();
    Buffer.BlockCopy(name10, 0, p, 5, Math.Min(10, name10.Length));

    p[15] = slot;
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(16, 2), level);
    p[18] = serverClass;
    return p;
}

static ReadOnlySpan<byte> TrimName10(ReadOnlySpan<byte> name10)
{
    var len = Math.Min(10, name10.Length);
    while (len > 0 && name10[len - 1] == 0)
    {
        len--;
    }

    return name10[..len];
}

static bool NameBytesEqual(ReadOnlySpan<byte> a, ReadOnlySpan<byte> b)
{
    return TrimName10(a).SequenceEqual(TrimName10(b));
}

static void UpsertRoster(List<CharacterRosterEntry> roster, byte[] name10, byte serverClass, ushort level)
{
    var copy = new byte[10];
    name10.AsSpan(0, Math.Min(10, name10.Length)).CopyTo(copy);
    for (var i = roster.Count - 1; i >= 0; i--)
    {
        if (NameBytesEqual(roster[i].Name10, copy))
        {
            roster.RemoveAt(i);
        }
    }

    roster.Add(new CharacterRosterEntry { Name10 = copy, ServerClass = serverClass, Level = level });
}

static CharacterRosterEntry? FindRosterEntry(List<CharacterRosterEntry> roster, byte[] joinName10)
{
    foreach (var e in roster)
    {
        if (NameBytesEqual(e.Name10, joinName10))
        {
            return e;
        }
    }

    return null;
}

/// <summary>
/// PMSG_REQUEST_JOIN_MAP_SERVER / Android <c>SendCharacterSelectionPacket</c>: C1 0E F3 (03|15) + name[10].
/// </summary>
static bool TryFindCharacterJoinRequest(ReadOnlySpan<byte> packet, out int frameOffset, out byte[] name10)
{
    frameOffset = -1;
    name10 = Array.Empty<byte>();
    Span<byte> scratch = stackalloc byte[14];

    for (var i = 0; i <= packet.Length - 14; i++)
    {
        if (packet[i] != 0xC1 || packet[i + 1] != 0x0E)
        {
            continue;
        }

        packet.Slice(i, 14).CopyTo(scratch);
        if (scratch[2] != 0xF3)
        {
            continue;
        }

        for (var pass = 0; pass < 8 && scratch[3] != 0x03 && scratch[3] != 0x15; pass++)
        {
            DecodeTakumiStreamXor(scratch, 3);
        }

        if (scratch[2] != 0xF3 || (scratch[3] != 0x03 && scratch[3] != 0x15))
        {
            continue;
        }

        frameOffset = i;
        name10 = new byte[10];
        scratch.Slice(4, 10).CopyTo(name10);
        return true;
    }

    return false;
}

/// <summary>Stderr-only: compare with adb <c>[AndroidLogin] recv tcp ... b0..b7=</c> for the same connection.</summary>
static void LogCharacterListWire(string remote, byte[] list, string tag)
{
    if (list.Length < 4)
    {
        Console.Error.WriteLine("[wire] {0} F3 00 {1}: len={2} (short packet)", remote, tag, list.Length);
        return;
    }

    var previewLen = Math.Min(24, list.Length);
    var preview = Convert.ToHexString(list.AsSpan(0, previewLen));
    var lenByte = list[1];
    Console.Error.WriteLine(
        "[wire] {0} F3 00 {1}: totalTcp={2} c1LenByte=0x{3:X2} head=0x{4:X2}{5:X2} preview={6}",
        remote,
        tag,
        list.Length,
        lenByte,
        list[2],
        list[3],
        preview);
    if (lenByte != list.Length)
    {
        Console.Error.WriteLine("[wire] WARNING: C1 length byte != buffer size");
    }
}

/// <summary>Character list F3 00 with one or more Takumi PRECEIVE_CHARACTER_LIST entries (WSclient.h).</summary>
/// <remarks>
/// MSVC lays out <c>PRECEIVE_CHARACTER_LIST</c> with 1 padding byte after <c>ID[10]</c> before <c>WORD Level</c>
/// (WORD alignment) → <c>sizeof</c> is 34, not 33. Omitting that byte shifts Class/Equipment and corrupts select models.
/// </remarks>
static byte[] BuildCharacterList602(List<CharacterRosterEntry> roster)
{
    const int headerSize = 8;
    const int entrySize = 34;
    var n = roster.Count;
    var total = headerSize + n * entrySize;
    var p = new byte[total];
    p[0] = 0xC1;
    p[1] = (byte)total;
    p[2] = 0xF3;
    p[3] = 0x00;
    p[4] = 7; // MaxClass
    p[5] = 0; // MoveCount
    p[6] = (byte)n;
    p[7] = 0; // ExtWarehouse

    var off = headerSize;
    for (var i = 0; i < n; i++)
    {
        var e = roster[i];
        p[off++] = (byte)i;
        e.Name10.AsSpan(0, 10).CopyTo(p.AsSpan(off));
        off += 10;
        p[off++] = 0; // struct padding before Level (matches MSVC PRECEIVE_CHARACTER_LIST)
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(off), e.Level);
        off += 2;
        p[off++] = 0; // CtlCode
        p[off++] = e.ServerClass;
        for (var k = 0; k < 17; k++)
        {
            p[off++] = 0xFF;
        }

        p[off++] = 0; // byGuildStatus
    }

    return p;
}

/// <summary><c>PRECEIVE_JOIN_MAP_SERVER</c> (Takumi WSclient.h) — plain C1, same as other host→client stubs here.</summary>
static byte[] BuildJoinMapServer602(CharacterRosterEntry r)
{
    var p = new byte[123];
    p[0] = 0xC1;
    p[1] = 123;
    p[2] = 0xF3;
    p[3] = 0x03;
    // Lorencia (0) — safe starter tile near center.
    p[4] = 135;
    p[5] = 122;
    p[6] = 0;
    p[7] = 1; // angle byte 1 → 0° in client ((Angle-1)*45)

    BinaryPrimitives.WriteUInt64BigEndian(p.AsSpan(8), (ulong)Math.Max(0, (r.Level - 1) * 50));
    BinaryPrimitives.WriteUInt64BigEndian(p.AsSpan(16), 200);

    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(24), (ushort)(r.Level > 1 ? r.Level - 1 : 0));
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(26), 18);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(28), 18);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(30), 15);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(32), 30);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(34), 60);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(36), 60);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(38), 46);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(40), 46);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(42), 0);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(44), 0);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(46), 0);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(48), 0);
    BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(50), 0);
    p[54] = 3;
    p[55] = 0;
    BinaryPrimitives.WriteInt16LittleEndian(p.AsSpan(56), 0);
    BinaryPrimitives.WriteInt16LittleEndian(p.AsSpan(58), 0);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(60), 0);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(62), 0);
    BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(64), 0);
    p[66] = 0;
    p.AsSpan(67, 56).Clear();
    return p;
}

/// <summary>Find PMSG_CREATE_CHARACTER (F3/01).</summary>
/// <remarks>
/// C++ path: StreamPacketEngine XOR (StreamPacketEngine.h) then SendGameEncrypted applies mu::Xor32Encrypt from index 3
/// (SimpleModulusCrypt.h — same 32-byte table). Client send log shows sub 0x0C for logical 0x01.
/// PipelinedDecryptor may leave bytes still obfuscated or fully logical; run Takumi decode only when subcode is not yet 0x01.
/// </remarks>
static bool TryFindCreateCharacterRequest(ReadOnlySpan<byte> packet, string remote, out int frameOffset, out byte[] name10, out byte serverClass)
{
    frameOffset = -1;
    name10 = Array.Empty<byte>();
    serverClass = 0;
    Span<byte> scratch = stackalloc byte[15];

    for (var i = 0; i <= packet.Length - 15; i++)
    {
        // Android JNI: SendCreateCharacter builds C1 0x0F F3 01 … then SendGameEncrypted (Xor32 only).
        // Desktop macro uses StreamPacketEngine XOR first; both end up as 15-byte PBMSG after decrypt + optional peels.
        if (packet[i] != 0xC1 || packet[i + 1] != 0x0F)
        {
            continue;
        }

        packet.Slice(i, 15).CopyTo(scratch);
        if (scratch[2] != 0xF3)
        {
            Console.WriteLine(
                "[{0}] create-candidate rejected — head@2=0x{1:X2} (want F3) hex={2}",
                remote,
                scratch[2],
                Convert.ToHexString(packet.Slice(i, 15)));
            continue;
        }

        // Client path: StreamPacketEngine XOR then SendGameEncrypted applies mu::Xor32Encrypt on the full buffer.
        // OpenMU PipelinedDecryptor strips one Xor32 layer; repeat inverse XOR while subcode ≠ logical 0x01 (max rounds guard).
        for (var pass = 0; pass < 8 && scratch[3] != 0x01; pass++)
        {
            DecodeTakumiStreamXor(scratch, firstXorIndex: 3);
        }

        if (scratch[3] != 0x01)
        {
            Console.WriteLine(
                "[{0}] create F3/01 still not normalized (sub@3=0x{1:X2}) — hex={2}",
                remote,
                scratch[3],
                Convert.ToHexString(packet.Slice(i, 15)));

            continue;
        }

        frameOffset = i;
        name10 = new byte[10];
        scratch.Slice(4, 10).CopyTo(name10);
        serverClass = scratch[14];
        return true;
    }

    return false;
}

/// <summary>Detects PMSG_CHARACTER_LIST_REQ (F3 / 00). Takumi XOR hides logical 0x00 (wire often 0x0D after F3).</summary>
static bool TryFindCharacterListRequest(ReadOnlySpan<byte> packet, out int frameOffset)
{
    frameOffset = -1;
    Span<byte> scratch = stackalloc byte[8];

    // PBMSG: Code, Size, HeadCode, SubCode — plain MU or Takumi stream-XOR on tail.
    for (var i = 0; i <= packet.Length - 4; i++)
    {
        if (packet[i] is not (0xC1 or 0xC3))
        {
            continue;
        }

        if (packet[i + 2] == 0xF3 && packet[i + 3] == 0x00)
        {
            frameOffset = i;
            return true;
        }

        // F3/00 list req: wire sub often 0x0D (Takumi XOR of 0x00); OpenMU may already expose 0x00.
        if (packet[i] == 0xC1 && packet[i + 2] == 0xF3 && i + 5 <= packet.Length)
        {
            packet.Slice(i, 5).CopyTo(scratch[..5]);
            if (scratch[3] != 0x00)
            {
                DecodeTakumiStreamXor(scratch[..5], firstXorIndex: 3);
            }

            if (scratch[3] == 0x00)
            {
                frameOffset = i;
                return true;
            }
        }
    }

    // C3 + SimpleModulus counter: Code, Size, Counter/serial, Head, Sub — Head F3 Sub 00 at +3,+4 (see OpenMU packet docs).
    for (var i = 0; i <= packet.Length - 5; i++)
    {
        if (packet[i] != 0xC3)
        {
            continue;
        }

        if (packet[i + 3] == 0xF3 && packet[i + 4] == 0x00)
        {
            frameOffset = i;
            return true;
        }
    }

    // Size byte + F3 00 still preceded by C1/C3 two bytes earlier.
    for (var i = 0; i <= packet.Length - 3; i++)
    {
        if (packet[i] != 0x05 || packet[i + 1] != 0xF3 || packet[i + 2] != 0x00)
        {
            continue;
        }

        if (i >= 2 && packet[i - 2] is 0xC1 or 0xC3)
        {
            frameOffset = i - 2;
            return true;
        }
    }

    // Short post-login control packets: list request is the usual F3/00 op (see WSclient ReceiveCharacterList path).
    // Do not treat PMSG_CREATE_CHARACTER (C1 0x0F F3 xx, 15 bytes) as list because payload may contain 0xF3 0x00
    // (e.g. last name byte + class 0).
    if (packet.Length <= 16)
    {
        var skipSubstringScan = packet.Length == 15
            && packet[0] == 0xC1
            && packet[1] == 0x0F
            && packet[2] == 0xF3;
        if (!skipSubstringScan)
        {
            for (var j = 0; j <= packet.Length - 2; j++)
            {
                if (packet[j] == 0xF3 && packet[j + 1] == 0x00)
                {
                    frameOffset = j >= 2 ? j - 2 : 0;
                    return true;
                }
            }
        }
    }

    return false;
}

static Task WriteLoginResultAsync(Connection connection, byte result, CancellationToken ct)
{
    var pkt = BuildLoginResultPacket(result);
    return WriteAsync(connection, pkt, ct);
}

static async Task WriteAsync(Connection connection, ReadOnlyMemory<byte> pkt, CancellationToken ct)
{
    await connection.Output.WriteAsync(pkt, ct).ConfigureAwait(false);
    await connection.Output.FlushAsync(ct).ConfigureAwait(false);
}

static (SimpleModulusKeys Keys, string SourceTag) LoadDecryptKeysFromDec2OrDefault()
{
    var serializer = new SimpleModulusKeySerializer();
    var candidates = new List<string>();

    var envPath = Environment.GetEnvironmentVariable("TAKUMI_DEC2_PATH");
    if (!string.IsNullOrWhiteSpace(envPath))
    {
        var trimmed = envPath.Trim();
        candidates.Add(trimmed);
        if (!File.Exists(trimmed))
        {
            Console.Error.WriteLine(
                "[keys] ERROR: TAKUMI_DEC2_PATH is set but file does not exist:\n  {0}\n" +
                "Use a real path (copy from phone: adb pull …/files/Data/Dec2.dat). Do not use placeholder text.",
                Path.GetFullPath(trimmed));
        }
    }

    candidates.Add(Path.Combine(AppContext.BaseDirectory, "Data", "Dec2.dat"));
    candidates.Add(Path.Combine(Environment.CurrentDirectory, "Data", "Dec2.dat"));
    candidates.Add(Path.Combine(Environment.CurrentDirectory, "Dec2.dat"));

    foreach (var dir in new[] { AppContext.BaseDirectory, Environment.CurrentDirectory })
    {
        try
        {
            if (!string.IsNullOrEmpty(dir))
            {
                candidates.AddRange(WalkParentsForDataDec2(dir));
            }
        }
        catch
        {
            // ignore invalid paths
        }
    }

    foreach (var path in candidates.Distinct(StringComparer.OrdinalIgnoreCase))
    {
        try
        {
            if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
            {
                continue;
            }

            if (!serializer.TryDeserialize(path, out var modulusKey, out var cryptKey, out var xorKey))
            {
                Console.WriteLine("[keys] TryDeserialize failed for: {0}", path);
                continue;
            }

            if (modulusKey.Length != 4 || cryptKey.Length != 4 || xorKey.Length != 4)
            {
                Console.WriteLine(
                    "[keys] Unexpected lengths in {0}: mod={1} key={2} xor={3}",
                    path,
                    modulusKey.Length,
                    cryptKey.Length,
                    xorKey.Length);
                continue;
            }

            var combined = new uint[12];
            Array.Copy(modulusKey, 0, combined, 0, 4);
            Array.Copy(cryptKey, 0, combined, 4, 4);
            Array.Copy(xorKey, 0, combined, 8, 4);

            var keys = SimpleModulusKeys.CreateDecryptionKeys(combined);
            Console.WriteLine("[keys] Loaded Dec2.dat (server decrypt): {0}", path);
            return (keys, $"Dec2 ({path})");
        }
        catch (Exception ex)
        {
            Console.WriteLine("[keys] Error reading {0}: {1}", path, ex.Message);
        }
    }

    Console.WriteLine(
        "[keys] WARNING: Dec2.dat not loaded — using OpenMU default server keys. " +
        "If login never completes, copy the client Data/Dec2.dat beside the exe or set TAKUMI_DEC2_PATH.");
    return (PipelinedSimpleModulusDecryptor.DefaultServerKey, "OpenMU default (may NOT match your client)");
}

/// <summary>
/// Looks for Data/Dec2.dat walking up from <paramref name="startDir"/> (helps when running from bin/Release/net10.0).
/// </summary>
static IEnumerable<string> WalkParentsForDataDec2(string startDir)
{
    var dir = Path.GetFullPath(startDir);
    for (var depth = 0; depth < 18 && !string.IsNullOrEmpty(dir); depth++)
    {
        yield return Path.Combine(dir, "Data", "Dec2.dat");
        dir = Directory.GetParent(dir)?.FullName ?? string.Empty;
    }
}

static void BuxXor(byte[] buf)
{
    ReadOnlySpan<byte> xorTable = stackalloc byte[] { 0xFC, 0xCF, 0xAB };
    for (var i = 0; i < buf.Length; i++)
    {
        buf[i] ^= xorTable[i % 3];
    }
}

static byte[]? ParseHexOrAscii5(string? s)
{
    if (string.IsNullOrWhiteSpace(s))
    {
        return null;
    }

    s = s.Trim();
    if (s.Length == 5 && s.All(c => c < 128))
    {
        return Encoding.ASCII.GetBytes(s);
    }

    var parts = s.Split(new[] { ' ', ',', ';' }, StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
    if (parts.Length == 5 && parts.All(p => byte.TryParse(p, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out _)))
    {
        return parts.Select(p => byte.Parse(p, NumberStyles.HexNumber, CultureInfo.InvariantCulture)).ToArray();
    }

    if (parts.Length == 5)
    {
        try
        {
            return Convert.FromHexString(string.Concat(parts));
        }
        catch
        {
            return null;
        }
    }

    try
    {
        var bytes = Convert.FromHexString(s.Replace(" ", string.Empty, StringComparison.Ordinal));
        return bytes.Length == 5 ? bytes : null;
    }
    catch
    {
        return null;
    }
}

static byte[]? ParseSerial(string? s)
{
    if (string.IsNullOrWhiteSpace(s))
    {
        return null;
    }

    var t = s.Trim();
    if (t.Length == 16 && t.All(static c => c < 128))
    {
        return Encoding.ASCII.GetBytes(t);
    }

    return null;
}

static Dictionary<string, string>? ParseAccounts(string? s)
{
    if (string.IsNullOrWhiteSpace(s))
    {
        return null;
    }

    var d = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
    foreach (var part in s.Split(new[] { ';', '|' }, StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
    {
        var idx = part.IndexOf(':');
        if (idx <= 0)
        {
            continue;
        }

        d[part[..idx].Trim()] = part[(idx + 1)..].Trim();
    }

    return d.Count > 0 ? d : null;
}

/// <summary>Wire constants for minimal Connect Server (same process as login).</summary>
file static class RosterIo
{
    internal static readonly object Lock = new();
    internal static readonly JsonSerializerOptions Json = new()
    {
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        WriteIndented = true,
    };
}

file sealed class CharacterRosterEntry
{
    public byte[] Name10 = new byte[10];
    public byte ServerClass;
    public ushort Level;
}

file sealed class RosterPersistRoot
{
    public List<RosterPersistChar> Characters { get; set; } = new();
}

file sealed class RosterPersistChar
{
    public string Name { get; set; } = "";
    public byte ServerClass { get; set; }
    public ushort Level { get; set; }
}

file static class ConnectWire
{
    /// <summary>
    /// Season 6 connect-server list (C2 F4 06). Layout matches Takumi WSclient ReceiveServerList / OpenMU ServerListResponse.
    /// Each entry is 4 bytes: server id (LE), load %, padding. Client maps id → ServerList.bmd via index/20 (ServerListManager.cpp).
    /// </summary>
    public static byte[] BuildServerList602(int connectBase, int serverCount, byte loadPercent)
    {
        if (connectBase < 0 || connectBase > 65535)
        {
            throw new ArgumentOutOfRangeException(nameof(connectBase));
        }

        if (serverCount is < 1 or > 32)
        {
            throw new ArgumentOutOfRangeException(nameof(serverCount));
        }

        var len = 7 + serverCount * 4;
        var p = new byte[len];
        p[0] = 0xC2;
        BinaryPrimitives.WriteUInt16BigEndian(p.AsSpan(1, 2), (ushort)len);
        p[3] = 0xF4;
        p[4] = 0x06;
        BinaryPrimitives.WriteUInt16BigEndian(p.AsSpan(5, 2), (ushort)serverCount);
        var off = 7;
        for (var i = 0; i < serverCount; i++)
        {
            var id = connectBase + i;
            if (id > 65535)
            {
                throw new ArgumentException("connectBase + count exceeds ushort server id range.", nameof(connectBase));
            }

            BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(off, 2), (ushort)id);
            p[off + 2] = loadPercent;
            p[off + 3] = 0;
            off += 4;
        }

        return p;
    }
}
