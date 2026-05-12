// Minimal Takumi / Season6-style login listener for Android JNI path:
// - Send plain C1 F1 00 (join) immediately after TCP accept (same as GameServer GCConnectClientSend).
// - Decrypt client->server with OpenMU PipelinedDecryptor (SimpleModulus then Xor32), matching Android SendGameEncrypted.
//   IMPORTANT: keys must match the client's Data/Dec2.dat (set TAKUMI_DEC2_PATH or place Data/Dec2.dat next to the exe).
// - Parse C3/C1 F1 01 login, validate version + serial + credentials, answer C1 F1 01, then C1 F3 00 (empty character list).

using System.Buffers;
using System.Buffers.Binary;
using System.Globalization;
using System.Net;
using System.Net.Sockets;
using System.Text;
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

using var cts = new CancellationTokenSource();
Console.CancelKeyPress += (_, e) => { e.Cancel = true; cts.Cancel(); };

TcpListener listener;
try
{
    listener = new TcpListener(IPAddress.Any, port);
    listener.Server.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
    listener.Start();
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
                await RunMinimalConnectServerAsync(connectPort, publicHost!, (ushort)port, verbose, cts.Token).ConfigureAwait(false);
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
        "Minimal Connect Server on *:{0} -> TAKUMI_PUBLIC_HOST={1} game/login port={2} (verbose={3})",
        connectPort,
        publicHost,
        port,
        verbose);
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

        _ = HandleClientAsync(tcp, joinVersion, serverSerial, accounts, serverDecryptKeys, cts.Token);
    }
}
finally
{
    listener.Stop();
}

return 0;

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

                var list = BuildEmptyCharacterList602();
                await connection.Output.WriteAsync(list, ct).ConfigureAwait(false);
                await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                Console.WriteLine(
                    "[{0}] sent character list F3 00 (empty) frameOffset={1} frameHead=0x{2:X2}",
                    remote,
                    listFrameOffset,
                    packet[listFrameOffset]);
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
            Console.WriteLine("[{0}] login ok id={1}", remote, id);
            // 0x01 and 0x20 are both treated as success in Takumi WSclient TranslateProtocol.
            await WriteLoginResultAsync(connection, 0x01, ct).ConfigureAwait(false);

            // Scene switches to character select and client sends F3 00; push empty list immediately as well (disable with TAKUMI_SKIP_AUTO_CHARLIST=1).
            if (!string.Equals(Environment.GetEnvironmentVariable("TAKUMI_SKIP_AUTO_CHARLIST"), "1", StringComparison.Ordinal))
            {
                var list = BuildEmptyCharacterList602();
                await connection.Output.WriteAsync(list, ct).ConfigureAwait(false);
                await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                Console.WriteLine("[{0}] sent empty character list F3 00 after login", remote);
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
    CancellationToken ct)
{
    var listener = new TcpListener(IPAddress.Any, listenPort);
    listener.Server.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
    listener.Start();
    Console.WriteLine("[connect] listening on *:{0}", listenPort);
    try
    {
        while (!ct.IsCancellationRequested)
        {
            var tcp = await listener.AcceptTcpClientAsync(ct).ConfigureAwait(false);
            _ = HandleMinimalConnectClientAsync(tcp, publicHost, gamePort, verbose, ct);
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
    CancellationToken ct)
{
    var remote = tcp.Client.RemoteEndPoint?.ToString() ?? "?";
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
                await stream.WriteAsync(ConnectWire.ServerList602, ct).ConfigureAwait(false);
                await stream.FlushAsync(ct).ConfigureAwait(false);
                Console.WriteLine("[connect] sent {0}: ServerList ({1} bytes)", remote, ConnectWire.ServerList602.Length);
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

static ReadOnlyMemory<byte> BuildEmptyCharacterList602()
{
    // PMSG_CHARACTER_LIST_SEND with ExtWarehouse (Season 6+ layout, 8 bytes total).
    return new byte[] { 0xC1, 0x08, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00 };
}

/// <summary>Detects PMSG_CHARACTER_LIST_REQ (F3 / 00). Layout varies after PipelinedDecryptor (counter / prefixes).</summary>
static bool TryFindCharacterListRequest(ReadOnlySpan<byte> packet, out int frameOffset)
{
    frameOffset = -1;

    // PBMSG: Code, Size, HeadCode, SubCode — standard client wire after XOR/SM.
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
    if (packet.Length <= 16)
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
file static class ConnectWire
{
    /// <summary>Season 6 server list (C2), captured from a working Takumi connect trace.</summary>
    public static readonly byte[] ServerList602 =
        Convert.FromHexString("C20013F406000300000A0001000A0002000A00");
}
