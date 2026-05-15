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
using Pipelines.Sockets.Unofficial;
using Takumi.Server.Connect;
using Takumi.Server.Join;
using Takumi.Server.Game;
using Takumi.Server.Game.World;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.LegacyLoginHost;

/// <summary>M5 split-process: login/game TCP only (no Connect mini-server on <c>TAKUMI_CONNECT_PORT</c>).</summary>
public enum LegacyLoginHostListenMode
{
    /// <summary>Read <c>TAKUMI_CONNECT_PORT</c> from environment (0 = disabled).</summary>
    RespectEnvironment,

    /// <summary>Never bind Connect — use a separate Connect process or combined <c>LegacyLoginHost</c>.</summary>
    LoginTcpOnly,
}

public sealed record LegacyLoginHostRunOptions(LegacyLoginHostListenMode ListenMode = LegacyLoginHostListenMode.RespectEnvironment);

public static class LegacyLoginHostRunner
{
    public static async Task<int> RunAsync(LegacyLoginHostRunOptions? runOptions, CancellationToken appCancellationToken)
    {
        TakumiPostgresMirror.InitIfEnabled();
        TakumiPostgresMirror.InitSessionHandoffIfEnabled();

        if (!int.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_LOGIN_PORT"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var port)
            || port is <= 0 or > 65535)
        {
            Console.Error.WriteLine(
                "Missing or invalid TAKUMI_LOGIN_PORT. Set it in server-next/env.defaults (committed) or server-next/.env (local).");
            return 1;
        }

        var advertisedGamePort = port;
        var gamePortRaw = Environment.GetEnvironmentVariable("TAKUMI_GAME_PORT")?.Trim();
        if (!string.IsNullOrEmpty(gamePortRaw)
            && int.TryParse(gamePortRaw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var gpParsed)
            && gpParsed is > 0 and <= 65535)
        {
            advertisedGamePort = gpParsed;
        }

        var joinVersion = GameWireEnv.ParseJoinVersion5(Environment.GetEnvironmentVariable("TAKUMI_JOIN_VERSION_HEX"))
                          ?? GameWireEnv.ParseJoinVersion5(Environment.GetEnvironmentVariable("TAKUMI_JOIN_VERSION"));
        if (joinVersion is null || joinVersion.Length != 5)
        {
            Console.Error.WriteLine(
                "Join version must be exactly 5 bytes (wire). Set TAKUMI_JOIN_VERSION=10405 or TAKUMI_JOIN_VERSION_HEX in server-next/env.defaults or .env.");
            return 1;
        }

        var serverSerial = GameWireEnv.ParseSerial16(Environment.GetEnvironmentVariable("TAKUMI_SERVER_SERIAL"));
        if (serverSerial is null || serverSerial.Length != 16)
        {
            Console.Error.WriteLine("Server serial must be 16 bytes ASCII. Set TAKUMI_SERVER_SERIAL in server-next/env.defaults or .env.");
            return 1;
        }

        var accounts = ParseAccounts(Environment.GetEnvironmentVariable("TAKUMI_ACCOUNTS"));
        if (accounts is null || accounts.Count == 0)
        {
            Console.Error.WriteLine(
                "Missing TAKUMI_ACCOUNTS (user:pass pairs, use | or ; between pairs). Set in server-next/env.defaults or .env.");
            return 1;
        }

        var (serverDecryptKeys, decryptKeysTag) = Season6ClientToServerDecryptSession.LoadServerDecryptKeysFromDec2OrEnv();

        var verbose = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_VERBOSE"), "1", StringComparison.OrdinalIgnoreCase)
                      || string.Equals(Environment.GetEnvironmentVariable("DOTNET_ENVIRONMENT"), "Development", StringComparison.OrdinalIgnoreCase);

        var sessionTicketStore = new InMemorySessionTicketStore();
        var sessionTicketTtl = ParseSessionTicketTtl();

        // Minimal Connect Server (F4 06 / F4 03) for Android LAN QA. Set TAKUMI_CONNECT_PORT=0 to disable.
        var connectPort = 0;
        if (!string.IsNullOrWhiteSpace(Environment.GetEnvironmentVariable("TAKUMI_CONNECT_PORT"))
            && int.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_CONNECT_PORT"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var cpEnv)
            && cpEnv is >= 0 and <= 65535)
        {
            connectPort = cpEnv;
        }

        if (runOptions?.ListenMode == LegacyLoginHostListenMode.LoginTcpOnly)
        {
            connectPort = 0;
        }

        var publicHost = Environment.GetEnvironmentVariable("TAKUMI_PUBLIC_HOST")?.Trim();
        if (string.IsNullOrEmpty(publicHost))
        {
            publicHost = Environment.GetEnvironmentVariable("TAKUMI_LAN_IP")?.Trim();
        }

        if (string.IsNullOrEmpty(publicHost))
        {
            Console.Error.WriteLine(
                "[connect] WARNING: TAKUMI_LAN_IP unset (optional override: TAKUMI_PUBLIC_HOST) — using 127.0.0.1. Phones on Wi‑Fi cannot reach that; set server-next/.env.");
            publicHost = "127.0.0.1";
        }

        // Connect-server list (C2 F4 06): Takumi maps connect index → ServerList.bmd group via (index/20) — see ServerListManager.cpp.
        // If every ID maps to a missing BMD group, InsertServerGroup drops the line → empty sub-server list.
        // Default F4 06 must not send >15 connect slots per BMD group: client InsertServer indexes
        // m_abyNonPvpServer[(connectIndex%20+1)-1] but SLM_MAX_SERVER_COUNT is 15 (ServerListManager.h) — ids with
        // connectIndex%20 >= 15 SIGSEGV. Safe pattern: per group g use wire ids g*20 .. g*20+14 only.
        // Override: TAKUMI_CS_CONNECT_IDS=…  OR  TAKUMI_CS_CONNECT_BASE + TAKUMI_CS_CONNECT_COUNT (sequential IDs).
        var csBaseRaw = Environment.GetEnvironmentVariable("TAKUMI_CS_CONNECT_BASE");
        var csCountRaw = Environment.GetEnvironmentVariable("TAKUMI_CS_CONNECT_COUNT");
        var csIdsRaw = Environment.GetEnvironmentVariable("TAKUMI_CS_CONNECT_IDS");

        byte[] connectServerListPacket;
        string connectListBootDesc;
        if (TryParseConnectIdsCsv(csIdsRaw, out var explicitConnectIds))
        {
            connectServerListPacket = ConnectServerList602.BuildFromIds(explicitConnectIds, loadPercent: 0x0A);
            connectListBootDesc = $"TAKUMI_CS_CONNECT_IDS=[{string.Join(',', explicitConnectIds)}]";
        }
        else if (!string.IsNullOrWhiteSpace(csBaseRaw) || !string.IsNullOrWhiteSpace(csCountRaw))
        {
            // NOTE: TAKUMI_CS_CONNECT_BASE=0 in .env is a common mistake (no group 0 in many BMDs).
            var csConnectBase = 20;
            if (!string.IsNullOrWhiteSpace(csBaseRaw)
                && int.TryParse(csBaseRaw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var csb))
            {
                if (csb is >= 1 and <= 65532)
                {
                    csConnectBase = csb;
                }
                else if (csb == 0)
                {
                    Console.Error.WriteLine(
                        "[connect] WARNING: TAKUMI_CS_CONNECT_BASE=0 is invalid for typical ServerList.bmd (no group 0). Using 20. Remove the line from .env or set 20/40/…");
                }
                else
                {
                    Console.Error.WriteLine(
                        "[connect] WARNING: TAKUMI_CS_CONNECT_BASE={0} out of range 1..65532 — using 20.",
                        csb);
                }
            }

            var csConnectCount = 3;
            if (!string.IsNullOrWhiteSpace(csCountRaw)
                && int.TryParse(csCountRaw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var csc)
                && csc is >= 1 and <= 32)
            {
                csConnectCount = csc;
            }

            connectServerListPacket = ConnectServerList602.Build(csConnectBase, csConnectCount, loadPercent: 0x0A);
            connectListBootDesc = $"TAKUMI_CS_CONNECT_BASE={csConnectBase} TAKUMI_CS_CONNECT_COUNT={csConnectCount}";
        }
        else
        {
            // 32 F4 06 slots max: 15 safe ids per group (see SLM_MAX_SERVER_COUNT=15 vs %20 slot math in Takumi InsertServer).
            // Group0: 0..14, group1: 20..34, group2: 40..41 — many subs without out-of-bounds NonPVP read.
            Span<int> preset = stackalloc int[32];
            var wi = 0;
            for (var j = 0; j < 15; j++)
            {
                preset[wi++] = j;
            }

            for (var j = 0; j < 15; j++)
            {
                preset[wi++] = 20 + j;
            }

            preset[wi++] = 40;
            preset[wi++] = 41;
            connectServerListPacket = ConnectServerList602.BuildFromIds(preset, loadPercent: 0x0A);
            connectListBootDesc =
                "default F4 06: 15+15+2 safe ids (0..14,20..34,40,41); never >15 slots/group (client SLM_MAX_SERVER_COUNT)";
        }

        var connectReturnBusyRaw = Environment.GetEnvironmentVariable("TAKUMI_CONNECT_RETURN_BUSY")?.Trim();
        var connectReturnBusy = string.Equals(connectReturnBusyRaw, "1", StringComparison.OrdinalIgnoreCase)
                                || string.Equals(connectReturnBusyRaw, "true", StringComparison.OrdinalIgnoreCase)
                                || string.Equals(connectReturnBusyRaw, "yes", StringComparison.OrdinalIgnoreCase);
        byte connectBusyServerIndex = 0;
        if (byte.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_CONNECT_BUSY_INDEX"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var cbi))
        {
            connectBusyServerIndex = cbi;
        }

        var connectSendListOnAcceptRaw = Environment.GetEnvironmentVariable("TAKUMI_CONNECT_SEND_LIST_ON_ACCEPT")?.Trim();
        var connectSendListOnExplicitOff = !string.IsNullOrWhiteSpace(connectSendListOnAcceptRaw)
            && (string.Equals(connectSendListOnAcceptRaw, "0", StringComparison.OrdinalIgnoreCase)
                || string.Equals(connectSendListOnAcceptRaw, "false", StringComparison.OrdinalIgnoreCase)
                || string.Equals(connectSendListOnAcceptRaw, "no", StringComparison.OrdinalIgnoreCase));
        var connectSendListOnAccept = !connectReturnBusy && !connectSendListOnExplicitOff;

        // SO_REUSEADDR can allow a *second* listener on the same port while an older host is still running (e.g. stray dotnet),
        // so phones may hit the old process and keep seeing C1 4A (33-byte slots). Default: strict bind (reuse off).
        var reuseSocketAddr = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_REUSE_ADDR"), "1", StringComparison.OrdinalIgnoreCase);

        using var cts = CancellationTokenSource.CreateLinkedTokenSource(appCancellationToken);
        Console.CancelKeyPress += (_, e) => { e.Cancel = true; cts.Cancel(); };

        TcpListener listener;
        TcpListener? connectListener = null;
        try
        {
            listener = new TcpListener(IPAddress.Any, port);
            listener.Server.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, reuseSocketAddr);
            listener.Start();
            Console.Error.WriteLine(
                "[boot] LegacyLoginHost pid={0} listenPort={1} connectF4GamePort={2} charlist slot bytes=34 (2 chars => wire C1 4C / 76 bytes). asm={3}\n" +
                "[boot] SO_REUSEADDR={4} (set TAKUMI_REUSE_ADDR=1 only if you hit TIME_WAIT bind failures after quick restarts)\n" +
                "[boot] If logcat still shows C1 4A / nbytes=74, another process may share this port — check: lsof -nP -iTCP:{1} -sTCP:LISTEN",
                Environment.ProcessId,
                port,
                advertisedGamePort,
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
            // Bind connect port on the main thread before any client can complete TCP handshake (see TAKUMI_CONNECT_PORT).
            // Previously Start() ran inside Task.Run, creating a short race where phones could connect before listen().
            try
            {
                connectListener = new TcpListener(IPAddress.Any, connectPort);
                connectListener.Server.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, reuseSocketAddr);
                connectListener.Start();
                var boundMsg = $"[connect] listening on *:{connectPort} (bound synchronously before game accept loop)";
                Console.WriteLine(boundMsg);
            }
            catch (SocketException ex) when (ex.SocketErrorCode == SocketError.AddressAlreadyInUse)
            {
                Console.Error.WriteLine(
                    "Cannot bind connect port {0}: address already in use. Stop the other listener or change TAKUMI_CONNECT_PORT / publish mapping.\n" +
                    "  Check: lsof -nP -iTCP:{0} -sTCP:LISTEN",
                    connectPort);
                return 1;
            }

            var connectOptions = new ConnectMiniServerOptions
            {
                PublicHost = publicHost!,
                GamePort = (ushort)advertisedGamePort,
                Verbose = verbose,
                ServerList602 = connectServerListPacket,
                SendServerListOnAccept = connectSendListOnAccept,
                ReturnBusy = connectReturnBusy,
                BusyServerIndex = connectBusyServerIndex,
            };

            _ = Task.Run(
                async () =>
                {
                    try
                    {
                        await ConnectMiniServer.RunAcceptLoopAsync(connectListener!, connectOptions, cts.Token).ConfigureAwait(false);
                    }
                    catch (OperationCanceledException) when (cts.IsCancellationRequested)
                    {
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
            "  Connect F4 03 game port:     {1} (set TAKUMI_GAME_PORT; default = login port)\n" +
            "  Join version (5 bytes wire): {2}\n" +
            "  Server serial (16 bytes):    {3}\n" +
            "  Accounts:                    {4}\n" +
            "  SimpleModulus (server decrypt): {5}\n" +
            "  Session ticket TTL:          TAKUMI_SESSION_TICKET_TTL_MINUTES (default 120)\n" +
            "  Game keepalive (post-login): TAKUMI_GAME_KEEPALIVE_SECONDS (default 25; 0=off)",
            port,
            advertisedGamePort,
            Convert.ToHexString(joinVersion),
            Encoding.ASCII.GetString(serverSerial),
            string.Join(", ", accounts.Select(kv => kv.Key + ":***")),
            decryptKeysTag);

        if (connectPort > 0)
        {
            var busyNote = connectReturnBusy
                ? $"  TAKUMI_CONNECT_RETURN_BUSY=1 -> list requests answer F4 05 busy (index={connectBusyServerIndex})\n"
                : string.Empty;
            var onAcceptNote = connectReturnBusy
                ? string.Empty
                : $"  C2 F4 06 push on TCP accept: {(connectSendListOnAccept ? "yes" : "no")} (TAKUMI_CONNECT_SEND_LIST_ON_ACCEPT=0 disables)\n";
            Console.WriteLine(
                "Minimal Connect Server on *:{0} -> TAKUMI_PUBLIC_HOST={1} F4 03 game port={2} (listen/login={3}) (verbose={4})\n" +
                "  F4 02|06 list + F4 03 info + patch/version (C1 head 0x02 or main 0x05)\n" +
                "{6}" +
                "{7}" +
                "  F4 06 payload: {5}",
                connectPort,
                publicHost,
                advertisedGamePort,
                port,
                verbose,
                connectListBootDesc,
                busyNote,
                onAcceptNote);
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
                var acceptMsg =
                    $"[game] TCP accept from {acceptedFrom} (listen {port}, F4 03 advertises {advertisedGamePort}) — next: join F1 00";
                Console.WriteLine(acceptMsg);
                Console.Error.WriteLine(acceptMsg);

                _ = HandleClientAsync(
                    tcp,
                    joinVersion,
                    serverSerial,
                    accounts,
                    serverDecryptKeys,
                    verbose,
                    sessionTicketStore,
                    sessionTicketTtl,
                    cts.Token);
            }
        }
        finally
        {
            try
            {
                connectListener?.Stop();
            }
            catch
            {
                // ignore double-stop / races during shutdown
            }

            listener.Stop();
        }

        return 0;
    }

    static bool TryParseConnectIdsCsv(string? csv, out int[] ids)
    {
        ids = Array.Empty<int>();
        if (string.IsNullOrWhiteSpace(csv))
        {
            return false;
        }

        var parts = csv.Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
        if (parts.Length is < 1 or > 32)
        {
            Console.Error.WriteLine("[connect] WARNING: TAKUMI_CS_CONNECT_IDS must have 1..32 comma-separated integers.");
            return false;
        }

        var list = new int[parts.Length];
        for (var i = 0; i < parts.Length; i++)
        {
            if (!int.TryParse(parts[i], NumberStyles.Integer, CultureInfo.InvariantCulture, out var id)
                || id is < 0 or > 65535)
            {
                Console.Error.WriteLine("[connect] WARNING: TAKUMI_CS_CONNECT_IDS invalid token '{0}'.", parts[i]);
                return false;
            }

            list[i] = id;
        }

        ids = list;
        return true;
    }

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

    static TimeSpan ParseSessionTicketTtl()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_SESSION_TICKET_TTL_MINUTES");
        if (string.IsNullOrWhiteSpace(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var min)
            || min <= 0)
        {
            return TimeSpan.FromMinutes(120);
        }

        min = Math.Clamp(min, 5, 7 * 24 * 60);
        return TimeSpan.FromMinutes(min);
    }

    static async Task HandleClientAsync(
        TcpClient tcp,
        byte[] joinVersion,
        byte[] serverSerial,
        IReadOnlyDictionary<string, string> accounts,
        SimpleModulusKeys serverDecryptKeys,
        bool verbose,
        InMemorySessionTicketStore sessionTicketStore,
        TimeSpan sessionTicketTtl,
        CancellationToken ct)
    {
        var remote = tcp.Client.RemoteEndPoint?.ToString() ?? "?";
        string? loggedAccountId = null;
        Guid? connectionSessionTicket = null;
        var roster = new List<CharacterRosterEntry>();
        var rosterDirty = 0;
        var rosterDbMergeOverlay =
            !string.Equals(Environment.GetEnvironmentVariable("TAKUMI_ROSTER_DB_MERGE_MODE")?.Trim(), "json", StringComparison.OrdinalIgnoreCase);
        byte[]? sessionJoinCharacterName10 = null;
        var monsterViewportTracker = new MonsterViewportTracker();
        try
        {
            tcp.NoDelay = true;
            var socket = tcp.Client;
            ConnectTcpKeepAlive.TryApply(socket);
            var socketConnection = SocketConnection.Create(socket);
            using var connection = new Connection(
                socketConnection,
                new PipelinedDecryptor(socketConnection.Input, serverDecryptKeys, DefaultKeys.Xor32Key),
                encryptionPipe: null,
                new NullLogger<Connection>());

            var join = LoginAccountWire602.BuildJoinPacket(result: 1, index: 0, joinVersion);
            await connection.Output.WriteAsync(join, ct).ConfigureAwait(false);
            await connection.Output.FlushAsync(ct).ConfigureAwait(false);
            // Stderr too — easier to spot when stdout is buffered or mixed with other hosts.
            var joinMsg = $"[{remote}] sent join C1 F1 00 ({join.Length} bytes) — client should log recv tcp first byte C1";
            Console.WriteLine(joinMsg);
            Console.Error.WriteLine(joinMsg);

            var loginLatch = new LoginLatch();
            // loggedAccountId, roster, sessionJoinCharacterName10 — outer scope for disconnect flush (M4).
            // PacketReceived may invoke handlers concurrently before the previous async handler awaits;
            // a follow-up 12-byte F3 request could run before SetLoggedIn() → list ignored. Serialize per connection.
            using var packetGate = new SemaphoreSlim(1, 1);
            var maxDecryptedPacketBytes = DecryptedPacketSafety602.ParseMaxDecryptedPacketBytes();
            var maxPacketsPerSecond = DecryptedPacketSafety602.ParseMaxPacketsPerSecond();
            var decryptedPacketRateGate = new DecryptedPacketRateGate(maxPacketsPerSecond);

            using var connectionCts = CancellationTokenSource.CreateLinkedTokenSource(ct);
            var keepAliveInterval = GamePortKeepAliveRunner.ParseIntervalSeconds();
            Task? keepAliveTask = null;
            if (keepAliveInterval > TimeSpan.Zero)
            {
                keepAliveTask = GamePortKeepAliveRunner.RunAsync(
                    connection,
                    packetGate,
                    () => loginLatch.IsLoggedIn,
                    tcp,
                    remote,
                    verbose,
                    keepAliveInterval,
                    clientProtectOutbound: null,
                    connectionCts.Token);
            }

            var periodicInterval = RosterPeriodicFlush.TryParseIntervalFromEnv();
            Task? rosterPeriodicTask = null;
            if (periodicInterval is { } per && per > TimeSpan.Zero)
            {
                rosterPeriodicTask = RosterPeriodicSaveRunner.RunAsync(
                    () => loginLatch.IsLoggedIn,
                    () => Volatile.Read(ref rosterDirty),
                    () => Volatile.Write(ref rosterDirty, 0),
                    () =>
                    {
                        if (!string.IsNullOrEmpty(loggedAccountId))
                        {
                            SavePersistedRoster(loggedAccountId, roster);
                        }
                    },
                    per,
                    connectionCts.Token);
            }

            connection.PacketReceived += OnPacketAsync;

            try
            {
                await connection.BeginReceiveAsync().ConfigureAwait(false);
            }
            finally
            {
                connectionCts.Cancel();
                if (keepAliveTask is not null)
                {
                    try
                    {
                        await keepAliveTask.ConfigureAwait(false);
                    }
                    catch
                    {
                    }
                }

                if (rosterPeriodicTask is not null)
                {
                    try
                    {
                        await rosterPeriodicTask.ConfigureAwait(false);
                    }
                    catch
                    {
                    }
                }
            }

            async ValueTask OnPacketAsync(ReadOnlySequence<byte> packetSeq)
            {
                await packetGate.WaitAsync(ct).ConfigureAwait(false);
                try
                {
                var plen = packetSeq.Length;
                if (plen > maxDecryptedPacketBytes)
                {
                    Console.WriteLine(
                        "[{0}] closing: decrypted packet too large len={1} max={2}",
                        remote,
                        plen,
                        maxDecryptedPacketBytes);
                    tcp.Dispose();
                    return;
                }

                if (maxPacketsPerSecond > 0 && !decryptedPacketRateGate.TryAllow(DateTimeOffset.UtcNow))
                {
                    Console.WriteLine(
                        "[{0}] closing: exceeded packet rate limit ({1}/s)",
                        remote,
                        maxPacketsPerSecond);
                    tcp.Dispose();
                    return;
                }

                var packet = packetSeq.ToArray();
                if (packet.Length == 0)
                {
                    return;
                }

                var t = packet[0];
                GameRxStructuredLog.DecryptedRx(remote, packet.Length, t, verbose);

                if (loginLatch.IsLoggedIn && packet.Length == 15 && packet[0] == 0xC1)
                {
                    Console.WriteLine(
                        "[{0}] trace C1×15 b1=0x{1:X2} b2=0x{2:X2} b3=0x{3:X2} hex={4}",
                        remote,
                        packet[1],
                        packet[2],
                        packet[3],
                        Convert.ToHexString(packet));
                }

                // Create character F3 01: C1 0F F3 01 + name[10] + packed class byte.
                // Client CharMakeWin uses SendRequestCreateCharacter: last byte = (CLASS_TYPE<<4)|skin (DK=1 → 0x1n), not raw protocol.
                // ReceiveCreateCharacter expects Data->Class = Season6 protocol (e.g. DK = 0x20) — see CharacterManager PROTO_CLASS_CODES.
                if (TryFindCreateCharacterRequest(packet, remote, out var createOff, out var nameCopy, out var packedClass))
                {
                    if (!loginLatch.IsLoggedIn)
                    {
                        Console.WriteLine("[{0}] create character (F3 01) before login — ignored", remote);
                        return;
                    }

                    var serverClass = CharacterCreateWire602.MapPackedClassToServerProtocol(packedClass);
                    UpsertRoster(roster, nameCopy, serverClass, level: 1);
                    var slotByte = (byte)(roster.Count - 1);
                    var resp = CharacterCreateWire602.BuildCreateSuccess(nameCopy, slotByte, level: 1, serverClass: serverClass);
                    await connection.Output.WriteAsync(resp, ct).ConfigureAwait(false);
                    await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                    if (!string.IsNullOrEmpty(loggedAccountId))
                    {
                        SavePersistedRoster(loggedAccountId, roster);
                    }

                    Console.WriteLine(
                        "[{0}] sent create character OK (F3 01) slot={1} packed=0x{2:X2} serverClass=0x{3:X2} name='{4}' frame@{5}",
                        remote,
                        slotByte,
                        packedClass,
                        serverClass,
                        Encoding.ASCII.GetString(nameCopy).TrimEnd('\0'),
                        createOff);
                    return;
                }

                // Join map BEFORE delete: Android join is C1 0E F3 15 + name (14 B). Delete is C1 22 F3 02 + … (34 B).
                // XOR-based delete detection must not run first — it can mis-read a join C3 blob as F3 02 and ACK delete.
                // Join map: desktop SendRequestJoinMapServer (F3 03) or Android SendSelectCharacter (F3 15) then ReceiveJoinMapServer.
                if (TryFindCharacterJoinRequest(packet, out var joinOff, out var joinName10))
                {
                    if (!loginLatch.IsLoggedIn)
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

                    var spawn = new JoinMapSpawnWire(picked.MapId, picked.PosX, picked.PosY, picked.Angle);
                    var joinPkt = JoinMapServerWire602.Build(ToWire(picked), spawn);
                    var invPkt = await JoinInventoryPacket602.BuildAsync(TakumiPostgresMirror.InventorySlots, loggedAccountId, joinName10, ct).ConfigureAwait(false);
                    await connection.Output.WriteAsync(joinPkt, ct).ConfigureAwait(false);
                    await connection.Output.WriteAsync(invPkt, ct).ConfigureAwait(false);
                    await MapMonsterScopeSender.TrySendAfterJoinAsync(
                        monsterViewportTracker,
                        connection,
                        clientProtectOutbound: null,
                        picked.MapId,
                        picked.PosX,
                        picked.PosY,
                        remote,
                        ct).ConfigureAwait(false);
                    await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                    if (JoinMapVitalsSeed.TryApplyFromJoinPacketIfUnset(picked.MaxHp > 0, joinPkt, out var joinVitals))
                    {
                        RosterVitalsLifecycle.ApplyVitals(
                            joinVitals,
                            ref picked.CurrentHp,
                            ref picked.MaxHp,
                            ref picked.CurrentMp,
                            ref picked.MaxMp,
                            ref picked.Zen);
                        Volatile.Write(ref rosterDirty, 1);
                    }

                    sessionJoinCharacterName10 = new byte[10];
                    Buffer.BlockCopy(joinName10, 0, sessionJoinCharacterName10, 0, 10);
                    Console.WriteLine(
                        "[{0}] sent join map (F3 03) + inventory (F3 10 len={8}) map={1} xy=({2},{3}) ang={4} name='{5}' rosterClass=0x{6:X2} frame@{7}",
                        remote,
                        joinPkt[6],
                        joinPkt[4],
                        joinPkt[5],
                        joinPkt[7],
                        Encoding.ASCII.GetString(picked.Name10).TrimEnd('\0'),
                        picked.ServerClass,
                        joinOff,
                        invPkt.Length);
                    if (connectionSessionTicket is { } tJoin)
                    {
                        sessionTicketStore.Touch(tJoin, sessionTicketTtl);
                        if (TakumiPostgresMirror.SessionHandoff is { } shJoin
                            && sessionTicketStore.TryValidate(tJoin, out _, out var expJoin))
                        {
                            try
                            {
                                await shJoin.TouchExpiresAsync(tJoin, expJoin, ct).ConfigureAwait(false);
                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine("[session-ticket-db] touch after join: {0}", ex.Message);
                            }
                        }
                    }

                    return;
                }

                // Delete character F3 02: C1 22 F3 02 + name[10] + resident[20] (Takumi `SendRequestDeleteCharacter` in wsclientinline.h).
                // Without this handler the client stays on MESSAGE_WAIT forever after OK on the captcha dialog.
                if (TryFindDeleteCharacterRequest(packet, out var deleteOff, out var deleteName10, out _))
                {
                    if (!loginLatch.IsLoggedIn)
                    {
                        Console.WriteLine("[{0}] delete character (F3 02) before login — ignored", remote);
                        return;
                    }

                    var pickedDel = FindRosterEntry(roster, deleteName10);
                    if (pickedDel is null)
                    {
                        Console.WriteLine(
                            "[{0}] delete character — not in roster name='{1}' (respond Value=2 resident wrong) frame@{2}",
                            remote,
                            Encoding.ASCII.GetString(deleteName10).TrimEnd('\0'),
                            deleteOff);
                        await connection.Output.WriteAsync(CharacterCreateWire602.BuildDeleteResponse(2), ct).ConfigureAwait(false);
                        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                        return;
                    }

                    var removed = roster.RemoveAll(e => NameBytesEqual(e.Name10, deleteName10));
                    if (!string.IsNullOrEmpty(loggedAccountId) && TakumiPostgresMirror.CharacterRoster is { } dbDel)
                    {
                        try
                        {
                            var delName = CharacterRosterMerge.NormaliseName(Encoding.ASCII.GetString(deleteName10));
                            await dbDel.DeleteCharacterAsync(loggedAccountId, delName, ct).ConfigureAwait(false);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("[roster-db] explicit delete row failed for {0}: {1}", loggedAccountId, ex.Message);
                        }
                    }

                    if (!string.IsNullOrEmpty(loggedAccountId))
                    {
                        SavePersistedRoster(loggedAccountId, roster);
                    }

                    await connection.Output.WriteAsync(CharacterCreateWire602.BuildDeleteResponse(1), ct).ConfigureAwait(false);
                    await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                    Console.WriteLine(
                        "[{0}] delete character OK removed={1} name='{2}' rosterCount={3} frame@{4}",
                        remote,
                        removed,
                        Encoding.ASCII.GetString(deleteName10).TrimEnd('\0'),
                        roster.Count,
                        deleteOff);
                    return;
                }

                if (loginLatch.IsLoggedIn && TryFindGameLogoutRequest(packet, out var logoutOff, out var logoutFlag))
                {
                    var ack = new byte[] { 0xC1, 0x05, 0xF1, 0x02, logoutFlag };
                    await connection.Output.WriteAsync(ack, ct).ConfigureAwait(false);
                    await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                    Console.WriteLine("[{0}] ack game logout F1 02 value=0x{1:X2} frame@{2}", remote, logoutFlag, logoutOff);
                    return;
                }

                if (loginLatch.IsLoggedIn
                    && sessionJoinCharacterName10 is not null
                    && TryFindMoveMapRequest(packet, out var moveOff, out _, out var mapIdx))
                {
                    var pickedMove = FindRosterEntry(roster, sessionJoinCharacterName10);
                    var ackMove = new byte[] { 0xC1, 0x05, 0x8E, 0x03, 0x01 };
                    await connection.Output.WriteAsync(ackMove, ct).ConfigureAwait(false);
                    await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                    if (pickedMove is not null)
                    {
                        pickedMove.MapId = (byte)(mapIdx & 0xFF);

                        var mvSpawn = new JoinMapSpawnWire(pickedMove.MapId, pickedMove.PosX, pickedMove.PosY, pickedMove.Angle);
                        var joinPktMove = JoinMapServerWire602.Build(ToWire(pickedMove), mvSpawn);
                        if (JoinMapVitalsSeed.TryApplyFromJoinPacketIfUnset(pickedMove.MaxHp > 0, joinPktMove, out var moveVitals))
                        {
                            RosterVitalsLifecycle.ApplyVitals(
                                moveVitals,
                                ref pickedMove.CurrentHp,
                                ref pickedMove.MaxHp,
                                ref pickedMove.CurrentMp,
                                ref pickedMove.MaxMp,
                                ref pickedMove.Zen);
                            Volatile.Write(ref rosterDirty, 1);
                        }

                        if (!string.IsNullOrEmpty(loggedAccountId))
                        {
                            SavePersistedRoster(loggedAccountId, roster);
                        }

                        var invMove = await JoinInventoryPacket602.BuildAsync(TakumiPostgresMirror.InventorySlots, loggedAccountId, sessionJoinCharacterName10, ct).ConfigureAwait(false);
                        await connection.Output.WriteAsync(joinPktMove, ct).ConfigureAwait(false);
                        await connection.Output.WriteAsync(invMove, ct).ConfigureAwait(false);
                        await MapMonsterScopeSender.TrySendAfterJoinAsync(
                            monsterViewportTracker,
                            connection,
                            clientProtectOutbound: null,
                            pickedMove.MapId,
                            pickedMove.PosX,
                            pickedMove.PosY,
                            remote,
                            ct).ConfigureAwait(false);
                        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                        Console.WriteLine(
                            "[{0}] stub move map: 8E 03 ok + F3 03 + F3 10 len={1} mapId={2} frame@{3}",
                            remote,
                            invMove.Length,
                            mapIdx,
                            moveOff);
                        if (connectionSessionTicket is { } tMove)
                        {
                            sessionTicketStore.Touch(tMove, sessionTicketTtl);
                            if (TakumiPostgresMirror.SessionHandoff is { } shMove
                                && sessionTicketStore.TryValidate(tMove, out _, out var expMove))
                            {
                                try
                                {
                                    await shMove.TouchExpiresAsync(tMove, expMove, ct).ConfigureAwait(false);
                                }
                                catch (Exception ex)
                                {
                                    Console.WriteLine("[session-ticket-db] touch after move-map: {0}", ex.Message);
                                }
                            }
                        }
                    }
                    else
                    {
                        Console.WriteLine("[{0}] stub move map: 8E 03 ok only (no roster match) frame@{1}", remote, moveOff);
                    }

                    return;
                }

                if (loginLatch.IsLoggedIn
                    && sessionJoinCharacterName10 is not null)
                {
                    var pickedCombat = FindRosterEntry(roster, sessionJoinCharacterName10);
                    if (pickedCombat is not null
                        && await MonsterCombatHandler.TryHandleCombatPacketAsync(
                            monsterViewportTracker,
                            connection,
                            clientProtectOutbound: null,
                            pickedCombat.MapId,
                            pickedCombat.PosX,
                            pickedCombat.PosY,
                            packet,
                            remote,
                            ct).ConfigureAwait(false))
                    {
                        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                        return;
                    }
                }

                if (loginLatch.IsLoggedIn
                    && sessionJoinCharacterName10 is not null
                    && ClientWalkPackets602.TryFindInstantMove(packet, out _, out var instX, out var instY))
                {
                    var pickedInst = FindRosterEntry(roster, sessionJoinCharacterName10);
                    if (pickedInst is not null)
                    {
                        pickedInst.PosX = instX;
                        pickedInst.PosY = instY;
                        Volatile.Write(ref rosterDirty, 1);
                        await MapMonsterScopeSender.TrySendOnMoveAsync(
                            monsterViewportTracker,
                            connection,
                            clientProtectOutbound: null,
                            pickedInst.MapId,
                            instX,
                            instY,
                            remote,
                            ct).ConfigureAwait(false);
                        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                    }

                    return;
                }

                if (loginLatch.IsLoggedIn
                    && sessionJoinCharacterName10 is not null
                    && ClientWalkPackets602.TryFindWalkEndTile(packet, out _, out var walkX, out var walkY, out var walkAng, out var walkMoved))
                {
                    var pickedWalk = FindRosterEntry(roster, sessionJoinCharacterName10);
                    if (pickedWalk is not null)
                    {
                        if (walkMoved)
                        {
                            pickedWalk.PosX = walkX;
                            pickedWalk.PosY = walkY;
                            await MapMonsterScopeSender.TrySendOnMoveAsync(
                                monsterViewportTracker,
                                connection,
                                clientProtectOutbound: null,
                                pickedWalk.MapId,
                                walkX,
                                walkY,
                                remote,
                                ct).ConfigureAwait(false);
                            await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                        }

                        pickedWalk.Angle = walkAng;
                        Volatile.Write(ref rosterDirty, 1);
                    }

                    return;
                }

                if (loginLatch.IsLoggedIn
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
                // Only after auth: F3 heuristics can match byte pairs inside large C3 F1:01 login (~90 B) and must not return early.
                var listFrameOffset = 0;
                var listReq = loginLatch.IsLoggedIn && TryFindCharacterListRequest(packet, out listFrameOffset);
                if (!listReq
                    && loginLatch.IsLoggedIn
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
                    var list = roster.Count > 0 ? CharacterListWire602.Build(MapRosterToWire(roster)) : CharacterListWire602.BuildEmpty();
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

                if (loginLatch.IsLoggedIn && packet.Length <= 24)
                {
                    Console.WriteLine("[{0}] post-login unmatched hex={1}", remote, Convert.ToHexString(packet));
                }

                // F1 01 account login: C3 + stream XOR peel (GamePacketFinders) — same path as GamePortMinimalSession.
                var loginFrame = packet;
                if (!loginLatch.IsLoggedIn
                    && GamePacketFinders.TryUnpackAccountLoginFrame(packet, out var unpackedLogin)
                    && unpackedLogin.Length >= 59)
                {
                    loginFrame = unpackedLogin;
                }
                else if (!loginLatch.IsLoggedIn && packet.Length >= 59 && packet[0] == 0xC1 && packet[2] == 0xF1)
                {
                    loginFrame = (byte[])packet.Clone();
                    var span = loginFrame.AsSpan();
                    for (var peel = 0; peel < 8 && span[3] != 0x01; peel++)
                    {
                        DecodeTakumiStreamXor(span, 3);
                    }
                }

                // Login: Android may send C3-wrapped F1 01; use first byte of the normalized frame (not raw packet[0]).
                byte head;
                byte sub;
                int payloadOffset;
                var loginLead = loginFrame[0];
                if (loginLead == 0xC3 && loginFrame.Length >= 4)
                {
                    head = loginFrame[2];
                    sub = loginFrame[3];
                    payloadOffset = 4;
                }
                else if (loginLead == 0xC1 && loginFrame.Length >= 4)
                {
                    head = loginFrame[2];
                    sub = loginFrame[3];
                    payloadOffset = 4;
                }
                else
                {
                    return;
                }

                // Android plain layout: ... account[10] password[20] tick[4] version[5] serial[16] (59 bytes after C3 header).
                if (head != 0xF1 || sub != 0x01 || loginFrame.Length < 59)
                {
                    if (loginLatch.IsLoggedIn
                        && loginFrame.Length <= 48
                        && string.Equals(Environment.GetEnvironmentVariable("TAKUMI_VERBOSE"), "1", StringComparison.OrdinalIgnoreCase))
                    {
                        Console.WriteLine(
                            "[{0}] not login pkt — hex={1} head@2=0x{2:X2} sub@3=0x{3:X2}",
                            remote,
                            Convert.ToHexString(loginFrame),
                            head,
                            sub);
                    }
                    else if (!loginLatch.IsLoggedIn && loginFrame.Length >= 40 && loginFrame[0] == 0xC1 && loginFrame[2] == 0xF1)
                    {
                        var previewLen = Math.Min(40, loginFrame.Length);
                        Console.WriteLine(
                            "[{0}] pre-login F1 frame not login (post-peel): sub@3=0x{1:X2} len={2} preview={3}",
                            remote,
                            loginFrame[3],
                            loginFrame.Length,
                            Convert.ToHexString(loginFrame.AsSpan(0, previewLen)));
                    }

                    return;
                }

                // Wire matches Source/5.Main wsclientinline.h SendRequestLogIn: id[10] password[20] tick[4] version[5] serial[16].
                var idEnc = loginFrame.AsSpan(payloadOffset, 10).ToArray();
                var passEnc = loginFrame.AsSpan(payloadOffset + 10, 20).ToArray();
                BuxXor(idEnc);
                BuxXor(passEnc);
                var id = Encoding.ASCII.GetString(idEnc).TrimEnd('\0', ' ');
                var pass = Encoding.ASCII.GetString(passEnc).TrimEnd('\0', ' ');

                var clientVer = loginFrame.AsSpan(payloadOffset + 34, 5);
                var clientSer = loginFrame.AsSpan(payloadOffset + 39, 16);

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

                loginLatch.SetLoggedIn();
                loggedAccountId = id;
                var issued = sessionTicketStore.Issue(id, sessionTicketTtl);
                connectionSessionTicket = issued.TicketId;
                if (TakumiPostgresMirror.SessionHandoff is { } shIss)
                {
                    try
                    {
                        await shIss.ReplacePendingForAccountAsync(
                                id,
                                issued.TicketId,
                                issued.ExpiresUtc,
                                ConnectClientIp.TryFormatIp(tcp.Client.RemoteEndPoint),
                                ct)
                            .ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("[session-ticket-db] replace pending failed: {0}", ex.Message);
                    }
                }

                if (verbose)
                {
                    Console.WriteLine(
                        "[{0}] M5 session ticket issued account={1} expUtc={2:o} ticket={3}",
                        remote,
                        id,
                        issued.ExpiresUtc,
                        issued.TicketId);
                }

                roster.Clear();
                LoadPersistedRoster(id, roster);
                if (rosterDbMergeOverlay && TakumiPostgresMirror.CharacterRoster is not null)
                {
                    try
                    {
                        var dbRows = await TakumiPostgresMirror.CharacterRoster.LoadByAccountAsync(id, ct).ConfigureAwait(false);
                        CharacterRosterMerge.ApplyDbOverlay(
                            roster,
                            dbRows,
                            static e => Encoding.ASCII.GetString(e.Name10).TrimEnd('\0', ' '),
                            static (e, d) =>
                            {
                                e.MapId = d.MapId;
                                e.PosX = d.PosX;
                                e.PosY = d.PosY;
                                e.Angle = d.Angle;
                                e.Level = d.Level;
                                e.ServerClass = d.ServerClass;
                                e.CurrentHp = d.CurrentHp;
                                e.MaxHp = d.MaxHp;
                                e.CurrentMp = d.CurrentMp;
                                e.MaxMp = d.MaxMp;
                                e.Zen = d.Zen;
                            });
                        CharacterRosterMirrorHealth.RecordMergeSuccess();
                    }
                    catch (Exception ex)
                    {
                        CharacterRosterMirrorHealth.RecordMergeFail();
                        Console.WriteLine("[roster-db] merge after login failed for {0}: {1}", id, ex.Message);
                    }
                }

                Console.WriteLine(
                    "[{0}] login ok id={1} rosterPersisted={2} rosterCount={3}",
                    remote,
                    id,
                    GetRosterFilePath(id),
                    roster.Count);
                // 0x01 and 0x20 are both treated as success in Takumi WSclient TranslateProtocol.
                await WriteLoginResultAsync(connection, 0x01, ct).ConfigureAwait(false);

                var hmacKeyPush = SessionTicketSignature602.ResolveHmacKeyFromEnv();
                if (hmacKeyPush.Length >= 8 && TakumiPostgresMirror.SessionHandoff is not null)
                {
                    var expUnix = issued.ExpiresUtc.ToUnixTimeSeconds();
                    Span<byte> acct10 = stackalloc byte[SessionTicketSignature602.AccountWireBytes];
                    SessionTicketSignature602.FormatAccount10(id, acct10);
                    var mac = SessionTicketSignature602.ComputeMacV1(hmacKeyPush, issued.TicketId, expUnix, acct10);
                    var push = SessionTicketWire602.BuildPushC1(issued.TicketId, expUnix, acct10, mac);
                    await connection.Output.WriteAsync(push, ct).ConfigureAwait(false);
                    await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                    Console.WriteLine("[{0}] sent F1 A5 session-ticket (wire) len={1}", remote, push.Length);
                }

                // Scene switches to character select and client sends F3 00; push list from disk (or empty) (disable with TAKUMI_SKIP_AUTO_CHARLIST=1).
                if (!string.Equals(Environment.GetEnvironmentVariable("TAKUMI_SKIP_AUTO_CHARLIST"), "1", StringComparison.Ordinal))
                {
                    var list = roster.Count > 0 ? CharacterListWire602.Build(MapRosterToWire(roster)) : CharacterListWire602.BuildEmpty();
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
            if (connectionSessionTicket is { } tid)
            {
                if (TakumiPostgresMirror.SessionHandoff is { } shFin)
                {
                    try
                    {
                        await shFin.DeleteByTicketAsync(tid, ct).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("[session-ticket-db] delete on disconnect: {0}", ex.Message);
                    }
                }

                sessionTicketStore.RevokeTicket(tid);
            }

            if (!string.IsNullOrEmpty(loggedAccountId) && roster.Count > 0)
            {
                try
                {
                    SavePersistedRoster(loggedAccountId, roster);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("[roster] disconnect flush failed: {0}", ex.Message);
                }
            }

            CharacterRosterMirrorWriter.TryDrainPendingUpserts(TimeSpan.FromMilliseconds(900));

            tcp.Dispose();
        }
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

    static JoinMapSpawnWire ReadNewCharacterSpawnDefaultsFromEnv()
    {
        var d = JoinMapSpawnWire.LorenciaDefault;
        if (byte.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_MAP"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var m))
        {
            d = d with { Map = m };
        }

        if (byte.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_X"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var x))
        {
            d = d with { PositionX = x };
        }

        if (byte.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_Y"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var y))
        {
            d = d with { PositionY = y };
        }

        if (byte.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_ANGLE"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var a)
            && a > 0)
        {
            d = d with { Angle = a };
        }

        return d;
    }

    static void ApplyLegacySpawnIfUnset(CharacterRosterEntry e)
    {
        if (e.PosX != 0 || e.PosY != 0 || e.Angle != 0)
        {
            return;
        }

        var d = ReadNewCharacterSpawnDefaultsFromEnv();
        e.MapId = d.Map;
        e.PosX = d.PositionX;
        e.PosY = d.PositionY;
        e.Angle = d.Angle;
    }

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
                var entry = new CharacterRosterEntry
                {
                    Name10 = nm,
                    ServerClass = c.ServerClass,
                    Level = c.Level,
                    MapId = c.MapId,
                    PosX = c.PosX,
                    PosY = c.PosY,
                    Angle = c.Angle,
                    CurrentHp = c.CurrentHp,
                    MaxHp = c.MaxHp,
                    CurrentMp = c.CurrentMp,
                    MaxMp = c.MaxMp,
                    Zen = c.Zen,
                };
                ApplyLegacySpawnIfUnset(entry);
                roster.Add(entry);
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

            root.Characters.Add(
                new RosterPersistChar
                {
                    Name = name,
                    ServerClass = e.ServerClass,
                    Level = e.Level,
                    MapId = e.MapId,
                    PosX = e.PosX,
                    PosY = e.PosY,
                    Angle = e.Angle,
                    CurrentHp = e.CurrentHp,
                    MaxHp = e.MaxHp,
                    CurrentMp = e.CurrentMp,
                    MaxMp = e.MaxMp,
                    Zen = e.Zen,
                });
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

        ScheduleRosterDbUpsert(accountId, roster);
    }

    static void ScheduleRosterDbUpsert(string accountId, IReadOnlyList<CharacterRosterEntry> roster)
    {
        CharacterRosterRow[] snapshot;
        try
        {
            var list = new List<CharacterRosterRow>(roster.Count);
            foreach (var e in roster)
            {
                var name = Encoding.ASCII.GetString(e.Name10).TrimEnd('\0', ' ');
                if (string.IsNullOrEmpty(name))
                {
                    continue;
                }

                list.Add(
                    CharacterRosterRowMapping.ToRow(
                        name,
                        e.ServerClass,
                        e.Level,
                        e.MapId,
                        e.PosX,
                        e.PosY,
                        e.Angle,
                        e.CurrentHp,
                        e.MaxHp,
                        e.CurrentMp,
                        e.MaxMp,
                        e.Zen));
            }

            snapshot = list.ToArray();
        }
        catch
        {
            return;
        }

        CharacterRosterMirrorWriter.ScheduleReplaceAccountRoster(accountId, snapshot);
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

        var sp = ReadNewCharacterSpawnDefaultsFromEnv();
        roster.Add(
            new CharacterRosterEntry
            {
                Name10 = copy,
                ServerClass = serverClass,
                Level = level,
                MapId = sp.Map,
                PosX = sp.PositionX,
                PosY = sp.PositionY,
                Angle = sp.Angle,
            });
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

    static CharacterRosterWire ToWire(CharacterRosterEntry e) =>
        new(e.Name10, e.ServerClass, e.Level, CharacterRosterVitals.FromInts(e.CurrentHp, e.MaxHp, e.CurrentMp, e.MaxMp, e.Zen));

    static List<CharacterRosterWire> MapRosterToWire(List<CharacterRosterEntry> roster)
    {
        var list = new List<CharacterRosterWire>(roster.Count);
        foreach (var e in roster)
        {
            list.Add(ToWire(e));
        }

        return list;
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

    /// <summary>Game client logout / return to char select (1) or server select (2): <c>C1|C3 ?? F1 02 &lt;flag&gt;</c> (Takumi <c>SendRequestLogOut</c> — <c>Send(TRUE)</c> uses C3 on wire).</summary>
    static bool TryFindGameLogoutRequest(ReadOnlySpan<byte> packet, out int frameOffset, out byte value)
    {
        frameOffset = -1;
        value = 0;
        Span<byte> xorScratch = stackalloc byte[64];
        for (var i = 0; i <= packet.Length - 5; i++)
        {
            var lead = packet[i];
            if (lead != 0xC1 && lead != 0xC3)
            {
                continue;
            }

            var size = packet[i + 1];
            if (size < 5 || i + size > packet.Length)
            {
                continue;
            }

            if (packet[i + 2] == 0xF1 && packet[i + 3] == 0x02)
            {
                frameOffset = i;
                value = packet[i + 4];
                return true;
            }

            // Still obfuscated after PipelinedDecryptor (same pattern as F3 join / create).
            if (lead == 0xC3 && size is >= 5 and <= 64 && i + size <= packet.Length)
            {
                var work = xorScratch[..(int)size];
                packet.Slice(i, size).CopyTo(work);
                for (var pass = 0; pass < 8 && (work[2] != 0xF1 || work[3] != 0x02); pass++)
                {
                    DecodeTakumiStreamXor(work, 3);
                }

                if (work[2] == 0xF1 && work[3] == 0x02)
                {
                    frameOffset = i;
                    value = work[4];
                    return true;
                }
            }
        }

        return false;
    }

    /// <summary>Move-map request: <c>C1|C3 0A 8E 02</c> + DWORD block key + WORD map index (<c>SendRequestMoveMap</c>).</summary>
    static bool TryFindMoveMapRequest(ReadOnlySpan<byte> packet, out int frameOffset, out uint blockKey, out ushort mapIndex)
    {
        frameOffset = -1;
        blockKey = 0;
        mapIndex = 0;
        for (var i = 0; i <= packet.Length - 10; i++)
        {
            var lead = packet[i];
            if (lead != 0xC1 && lead != 0xC3)
            {
                continue;
            }

            if (packet[i + 1] != 0x0A || packet[i + 2] != 0x8E || packet[i + 3] != 0x02)
            {
                continue;
            }

            frameOffset = i;
            blockKey = BinaryPrimitives.ReadUInt32LittleEndian(packet.Slice(i + 4));
            mapIndex = BinaryPrimitives.ReadUInt16LittleEndian(packet.Slice(i + 8));
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

    /// <summary>Find PMSG_CREATE_CHARACTER (F3/01).</summary>
    /// <remarks>
    /// C++ path: StreamPacketEngine XOR (StreamPacketEngine.h) then SendGameEncrypted applies mu::Xor32Encrypt from index 3
    /// (SimpleModulusCrypt.h — same 32-byte table). Client send log shows sub 0x0C for logical 0x01.
    /// PipelinedDecryptor may leave bytes still obfuscated or fully logical; run Takumi decode only when subcode is not yet 0x01.
    /// Last byte out param is the <b>packed</b> (UI job &lt;&lt; 4)|skin — map with <see cref="CharacterCreateWire602.MapPackedClassToServerProtocol"/> before roster/response.
    /// </remarks>
    static bool TryFindCreateCharacterRequest(ReadOnlySpan<byte> packet, string remote, out int frameOffset, out byte[] name10, out byte packedClass)
    {
        frameOffset = -1;
        name10 = Array.Empty<byte>();
        packedClass = 0;
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
            packedClass = scratch[14];
            return true;
        }

        return false;
    }

    /// <summary>Find PMSG_REQUEST_DELETE_CHARACTER (F3/02): C1 + Size + F3 + 02 + id[10] + resident[20] (34 bytes after full decrypt).</summary>
    /// <remarks>Delete ACK wire: <see cref="CharacterCreateWire602.BuildDeleteResponse"/>.</remarks>
    static bool TryFindDeleteCharacterRequest(ReadOnlySpan<byte> packet, out int frameOffset, out byte[] name10, out byte[] resident20)
    {
        frameOffset = -1;
        name10 = Array.Empty<byte>();
        resident20 = Array.Empty<byte>();
        const int kFrameLen = 34;
        Span<byte> scratch = stackalloc byte[kFrameLen];

        for (var i = 0; i <= packet.Length - kFrameLen; i++)
        {
            if (packet[i] != 0xC1)
            {
                continue;
            }

            // Fast path: plaintext delete — Takumi is always C1 0x22 (34 bytes). Reject C1 0E F3 15 (join) etc.
            if (packet[i + 1] == kFrameLen
                && packet[i + 2] == 0xF3
                && packet[i + 3] == 0x02
                && i + kFrameLen <= packet.Length)
            {
                frameOffset = i;
                name10 = packet.Slice(i + 4, 10).ToArray();
                resident20 = packet.Slice(i + 14, 20).ToArray();
                return true;
            }

            packet.Slice(i, kFrameLen).CopyTo(scratch);
            for (var pass = 0; pass < 8 && (scratch[2] != 0xF3 || scratch[3] != 0x02); pass++)
            {
                DecodeTakumiStreamXor(scratch, firstXorIndex: 3);
            }

            if (scratch[2] != 0xF3 || scratch[3] != 0x02)
            {
                continue;
            }

            // After XOR, require exact C1 length 0x22 — join/select is C1 0E (14); XOR must not yield a false delete.
            if (scratch[1] != kFrameLen)
            {
                continue;
            }

            frameOffset = i;
            name10 = new byte[10];
            scratch.Slice(4, 10).CopyTo(name10);
            resident20 = new byte[20];
            scratch.Slice(14, 20).CopyTo(resident20);
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

            // List requests are small; C3 account login is ~90 bytes and can contain incidental F3/00 at +2/+3.
            if (packet[i] == 0xC3 && (int)packet[i + 1] > 48)
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

            var c3Len = (int)packet[i + 1];
            if (c3Len is < 5 or > 48)
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
        var pkt = LoginAccountWire602.BuildLoginResult(result);
        return WriteAsync(connection, pkt, ct);
    }

    static async Task WriteAsync(Connection connection, ReadOnlyMemory<byte> pkt, CancellationToken ct)
    {
        await connection.Output.WriteAsync(pkt, ct).ConfigureAwait(false);
        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
    }

    static void BuxXor(byte[] buf)
    {
        ReadOnlySpan<byte> xorTable = stackalloc byte[] { 0xFC, 0xCF, 0xAB };
        for (var i = 0; i < buf.Length; i++)
        {
            buf[i] ^= xorTable[i % 3];
        }
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
}

internal static class RosterIo
{
    internal static readonly object Lock = new();
    internal static readonly JsonSerializerOptions Json = new()
    {
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        WriteIndented = true,
    };
}

internal sealed class CharacterRosterEntry
{
    public byte[] Name10 = new byte[10];
    public byte ServerClass;
    public ushort Level;

    /// <summary>World map id (Takumi <c>PRECEIVE_JOIN_MAP_SERVER.Map</c>).</summary>
    public byte MapId;

    public byte PosX;
    public byte PosY;

    /// <summary>1-based wire angle (client uses <c>(Angle-1)*45</c> degrees).</summary>
    public byte Angle;

    public int CurrentHp;
    public int MaxHp;
    public int CurrentMp;
    public int MaxMp;
    public long Zen;
}

internal sealed class RosterPersistRoot
{
    public List<RosterPersistChar> Characters { get; set; } = new();
}

internal sealed class RosterPersistChar
{
    public string Name { get; set; } = "";
    public byte ServerClass { get; set; }
    public ushort Level { get; set; }

    public byte MapId { get; set; }
    public byte PosX { get; set; }
    public byte PosY { get; set; }
    public byte Angle { get; set; }

    public int CurrentHp { get; set; }

    public int MaxHp { get; set; }

    public int CurrentMp { get; set; }

    public int MaxMp { get; set; }

    public long Zen { get; set; }
}

/// <summary>Thread-safe login flag for keepalive + packet gate (visibility across tasks).</summary>
internal sealed class LoginLatch
{
    private int _loggedIn;

    public bool IsLoggedIn => Volatile.Read(ref this._loggedIn) != 0;

    public void SetLoggedIn() => Volatile.Write(ref this._loggedIn, 1);
}

