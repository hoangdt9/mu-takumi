using System.Buffers;
using System.IO.Pipelines;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using System.Threading;
using Microsoft.Extensions.Logging;
using MUnique.OpenMU.Network;
using MUnique.OpenMU.Network.SimpleModulus;
using MUnique.OpenMU.Network.Xor;
using Pipelines.Sockets.Unofficial;
using Takumi.Server.Connect;
using Takumi.Server.Game.Networking;
using Takumi.Server.Game.World;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>
/// Minimal post-join game TCP: same login + character list + join map flow as <c>LegacyLoginHost</c> for clients
/// redirected by ConnectServer <c>F4 03</c> to <see cref="GamePortListenOptions.AuthAccounts"/> mode.
/// </summary>
public static class GamePortMinimalSession
{
    public static async Task RunAsync(
        TcpClient tcp,
        string remote,
        GamePortListenOptions options,
        CancellationToken ct)
    {
        var accounts = options.AuthAccounts ?? new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
        var serverSerial = options.AuthServerSerial16!;
        var joinVersion = options.JoinVersion5;
        var verbose = options.Verbose;

        string? loggedAccountId = null;
        string? wireVerifiedAccountNorm = null;
        var roster = new List<GameRosterEntry>();
        var rosterDirty = 0;
        var rosterDbMergeOverlay =
            !string.Equals(Environment.GetEnvironmentVariable("TAKUMI_ROSTER_DB_MERGE_MODE")?.Trim(), "json", StringComparison.OrdinalIgnoreCase);
        byte[]? sessionJoinCharacterName10 = null;
        var monsterViewportTracker = new MonsterViewportTracker();
        var presenceSessionId = Guid.NewGuid();
        var loginLatch = new LoginLatch();
        Task? protectInboundPumpTask = null;
        CancellationTokenSource? protectInboundPumpCts = null;

        void TrackVitalsOutbound(ReadOnlySpan<byte> span)
        {
            if (sessionJoinCharacterName10 is null)
            {
                return;
            }

            var active = FindRosterEntry(roster, sessionJoinCharacterName10);
            if (active is null)
            {
                return;
            }

            if (RosterVitalsOutboundTracker.TryApplyToGameEntry(active, span))
            {
                Volatile.Write(ref rosterDirty, 1);
            }
        }

        try
        {
            // Stdout marker: visible in `docker compose logs` even when stderr is noisy; bump suffix after RX-wire changes.
            Console.WriteLine(
                "[{0}] m6_minimal_session_begin hasClientProtectKeys={1} rxBuild=M6-2026-05-15c",
                remote,
                options.ClientProtectOutboundKeys.HasValue);
            tcp.NoDelay = true;
            ConnectTcpKeepAlive.TryApply(tcp.Client);
            var socketConnection = SocketConnection.Create(tcp.Client);
            PipeReader pipelinedDecryptInput = socketConnection.Input;
            if (options.ClientProtectOutboundKeys is { } wireProtectKeys)
            {
                // Android CWsctlc::sSend: SimpleModulus (SendPacket bEncrypt) builds C3 on the buffer, then
                // gProtect.EncryptData on the whole wire before RawSend — outer gProtect, inner SM. Strip gProtect
                // on raw TCP chunks before PipelinedDecryptor (SM then Xor32).
                protectInboundPumpCts = CancellationTokenSource.CreateLinkedTokenSource(ct);
                var inboundPipe = new Pipe();
                pipelinedDecryptInput = inboundPipe.Reader;
                protectInboundPumpTask = TakumiClientProtectInboundPump.RunAsync(
                    socketConnection.Input,
                    inboundPipe.Writer,
                    wireProtectKeys.EncDecKey1,
                    wireProtectKeys.EncDecKey2,
                    protectInboundPumpCts.Token);
                var pumpMsg = "[{0}] protect_inbound_pump on (gProtect strip before SM+XOR)";
                Console.WriteLine(pumpMsg, remote);
                Console.Error.WriteLine(pumpMsg, remote);
            }

            using var connLogFactory = LoggerFactory.Create(b =>
            {
                b.AddSimpleConsole(o =>
                {
                    o.SingleLine = true;
                    o.TimestampFormat = "HH:mm:ss ";
                });
                b.SetMinimumLevel(verbose ? LogLevel.Information : LogLevel.Warning);
            });
            var connectionLogger = connLogFactory.CreateLogger<Connection>();

            using var connection = new Connection(
                socketConnection,
                new PipelinedDecryptor(pipelinedDecryptInput, options.ServerDecryptKeys, DefaultKeys.Xor32Key),
                encryptionPipe: null,
                connectionLogger);

            var protect = options.ClientProtectOutboundKeys;
            var join = LoginAccountWire602.BuildJoinPacket(result: 1, options.JoinWireIndex, joinVersion);
            await GamePortOutboundWire.WriteAsync(connection, protect, join, ct).ConfigureAwait(false);
            var joinMsg = $"[{remote}] sent join C1 F1 00 ({join.Length} bytes) index={options.JoinWireIndex} (minimal-login)";
            Console.WriteLine(joinMsg);
            Console.Error.WriteLine(joinMsg);

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
                    protect,
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
                            SaveRoster(loggedAccountId, roster);
                        }
                    },
                    per,
                    connectionCts.Token);
            }

            connection.PacketReceived += OnPacketAsync;
            connection.Disconnected += OnConnDisconnectedAsync;

            try
            {
                await connection.BeginReceiveAsync().ConfigureAwait(false);
                Console.Error.WriteLine(
                    "[{0}] OpenMU BeginReceiveAsync returned (socket closed or decrypt loop stopped without exception)",
                    remote);
            }
            finally
            {
                connection.Disconnected -= OnConnDisconnectedAsync;
                connection.PacketReceived -= OnPacketAsync;
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

            ValueTask OnConnDisconnectedAsync()
            {
                Console.Error.WriteLine(
                    "[{0}] OpenMU Connection.Disconnected (invalid header / SM checksum / reset — see OpenMU log lines above)",
                    remote);
                return default;
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

                    if (!loginLatch.IsLoggedIn
                        && SessionTicketWire602.TryFindClientAttach(packet.AsSpan(), out var attachBody)
                        && SessionTicketWire602.TryReadBody(attachBody, out var tidWire, out var expUnixWire, out var acct10Wire, out var mac32Wire))
                    {
                        if (!options.RequireSignedSessionTicketWire)
                        {
                            if (verbose)
                            {
                                Console.WriteLine("[{0}] F1 A6 session-ticket attach ignored (TAKUMI_GAME_TICKET_WIRE off)", remote);
                            }

                            return;
                        }

                        var hmacKeyAttach = SessionTicketSignature602.ResolveHmacKeyFromEnv();
                        if (hmacKeyAttach.Length < 8)
                        {
                            Console.WriteLine("[{0}] F1 A6 rejected: TAKUMI_SESSION_TICKET_HMAC_KEY missing/short", remote);
                            return;
                        }

                        if (TakumiPostgresMirror.SessionHandoff is not { } shWire)
                        {
                            Console.WriteLine("[{0}] F1 A6 rejected: session handoff DB not enabled", remote);
                            return;
                        }

                        var acctFromAttach = Encoding.ASCII.GetString(acct10Wire).TrimEnd('\0', ' ');
                        var okAttach = await shWire.TryConsumeSignedWireAttachAsync(
                                tidWire,
                                acctFromAttach,
                                expUnixWire,
                                mac32Wire.ToArray(),
                                hmacKeyAttach.ToArray(),
                                ct)
                            .ConfigureAwait(false);
                        if (!okAttach)
                        {
                            Console.WriteLine(
                                "[{0}] F1 A6 attach rejected (MAC/row/expiry/account mismatch or already used)",
                                remote);
                            return;
                        }

                        wireVerifiedAccountNorm = CharacterRosterMerge.NormaliseName(acctFromAttach);
                        Console.WriteLine(
                            "[{0}] F1 A6 signed session-ticket verified accountNorm={1}",
                            remote,
                            wireVerifiedAccountNorm);
                        return;
                    }

                    if (!loginLatch.IsLoggedIn
                        && GameInGameRegistration.TryFindRequest(packet, out var regOff)
                        && GameInGameRegistration.TryParseRequest(packet, regOff, out var regReq))
                    {
                        var regResult = await AccountCredentialGate.RegisterAsync(regReq, accounts, ct).ConfigureAwait(false);
                        var regAck = GameInGameRegistration.BuildResponse(regResult);
                        await GamePortOutboundWire.WriteAsync(connection, protect, regAck, ct).ConfigureAwait(false);
                        Console.WriteLine(
                            "[{0}] in-game register account='{1}' result={2}",
                            remote,
                            regReq.Account,
                            regResult);
                        return;
                    }

                    if (GamePacketFinders.TryFindCreateCharacterRequest(packet, remote, verbose, out var createOff, out var nameCopy, out var packedClass))
                    {
                        if (!loginLatch.IsLoggedIn)
                        {
                            Console.WriteLine("[{0}] create character (F3 01) before login — ignored", remote);
                            return;
                        }

                        var serverClass = CharacterCreateWire602.MapPackedClassToServerProtocol(packedClass);
                        GameRosterMutations.UpsertNewCharacter(roster, nameCopy, serverClass, level: 1);
                        var slotByte = (byte)(roster.Count - 1);
                        var resp = CharacterCreateWire602.BuildCreateSuccess(nameCopy, slotByte, level: 1, serverClass: serverClass);
                        await GamePortOutboundWire.WriteAsync(connection, protect, resp, ct).ConfigureAwait(false);
                        if (!string.IsNullOrEmpty(loggedAccountId))
                        {
                            SaveRoster(loggedAccountId, roster);
                        }

                        Console.WriteLine(
                            "[{0}] sent create character OK (F3 01) slot={1} name='{2}' frame@{3}",
                            remote,
                            slotByte,
                            Encoding.ASCII.GetString(nameCopy).TrimEnd('\0'),
                            createOff);
                        return;
                    }

                    if (GamePacketFinders.TryFindDeleteCharacterRequest(packet, out var deleteOff, out var deleteName10, out var deleteResident20))
                    {
                        if (!loginLatch.IsLoggedIn)
                        {
                            Console.WriteLine("[{0}] delete character (F3 02) before login — ignored", remote);
                            return;
                        }

                        Console.WriteLine(
                            "[{0}] delete character request name='{1}' frame@{2} wireLen={3}",
                            remote,
                            Encoding.ASCII.GetString(deleteName10).TrimEnd('\0'),
                            deleteOff,
                            packet.Length);

                        var pickedDel = FindRosterEntry(roster, deleteName10);
                        if (pickedDel is null)
                        {
                            Console.WriteLine(
                                "[{0}] delete character — not in roster name='{1}' frame@{2}",
                                remote,
                                Encoding.ASCII.GetString(deleteName10).TrimEnd('\0'),
                                deleteOff);
                            await GamePortOutboundWire.WriteAsync(connection, protect, CharacterCreateWire602.BuildDeleteResponse(2), ct).ConfigureAwait(false);
                            return;
                        }

                        var removed = GameRosterMutations.RemoveByName(roster, deleteName10);
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
                            SaveRoster(loggedAccountId, roster);
                        }

                        await GamePortOutboundWire.WriteAsync(connection, protect, CharacterCreateWire602.BuildDeleteResponse(1), ct).ConfigureAwait(false);
                        Console.WriteLine(
                            "[{0}] delete character OK removed={1} name='{2}' rosterCount={3} frame@{4}",
                            remote,
                            removed,
                            Encoding.ASCII.GetString(deleteName10).TrimEnd('\0'),
                            roster.Count,
                            deleteOff);
                        return;
                    }

                    if (loginLatch.IsLoggedIn
                        && packet.Length == 34
                        && packet[0] == 0xC1
                        && !GamePacketFinders.TryFindDeleteCharacterRequest(packet, out _, out _, out _))
                    {
                        var previewLen = Math.Min(40, packet.Length);
                        Console.WriteLine(
                            "[{0}] delete F3 02 peel failed (len=34) preview={1}",
                            remote,
                            Convert.ToHexString(packet.AsSpan(0, previewLen)));
                    }

                    if (loginLatch.IsLoggedIn
                        && GamePacketFinders.TryFindCharacterJoinRequest(packet, out var joinOff, out var joinName10))
                    {
                        var picked = FindRosterEntry(roster, joinName10);
                        if (picked is null)
                        {
                            Console.WriteLine(
                                "[{0}] join map ignored — no roster match name='{1}'",
                                remote,
                                Encoding.ASCII.GetString(joinName10).TrimEnd('\0'));
                            return;
                        }

                        PlayerWalkHandler.HealSpawnTile(picked);
                        var spawn = new JoinMapSpawnWire(picked.MapId, picked.PosX, picked.PosY, picked.Angle);
                        var joinPkt = JoinMapServerWire602.Build(picked.ToWireWithSheet(), spawn);
                        var invPkt = await JoinInventoryLifecycle.BuildJoinPacketAsync(
                                TakumiPostgresMirror.InventorySlots,
                                loggedAccountId,
                                joinName10,
                                presenceSessionId,
                                ct)
                            .ConfigureAwait(false);
                        await GamePortOutboundWire.WriteAsync(connection, protect, joinPkt, ct, TrackVitalsOutbound).ConfigureAwait(false);
                        await GamePortOutboundWire.WriteAsync(connection, protect, invPkt, ct, TrackVitalsOutbound).ConfigureAwait(false);
                        await GamePortOutboundWire.WriteAsync(
                                connection,
                                protect,
                                MagicListWire602.BuildForServerClass(picked.ServerClass),
                                ct,
                                TrackVitalsOutbound)
                            .ConfigureAwait(false);
                        if (RosterVitalsLifecycle.SyncGameEntryFromJoin(picked, joinPkt))
                        {
                            Volatile.Write(ref rosterDirty, 1);
                        }

                        sessionJoinCharacterName10 = new byte[10];
                        Buffer.BlockCopy(joinName10, 0, sessionJoinCharacterName10, 0, 10);
                        var (bpCur, bpMax) = picked.ResolveBpForSync();
                        var computedVitals = CharacterSheetCalculator.ComputeMaxVitals(
                            picked.ServerClass,
                            picked.Level,
                            picked.ResolveSheet());
                        await RosterVitalsLifecycle.TrySendLifeManaSyncAsync(
                            async (m, t) => await GamePortOutboundWire.WriteAsync(connection, protect, m, t, TrackVitalsOutbound).ConfigureAwait(false),
                            picked.CurrentHp,
                            picked.MaxHp,
                            picked.CurrentMp,
                            picked.MaxMp,
                            ct,
                            picked.CurrentShield,
                            picked.MaxShield,
                            bpCur,
                            bpMax,
                            computedVitals).ConfigureAwait(false);
                        await GamePortOutboundWire.WriteAsync(
                                connection,
                                protect,
                                NewCharacterCalcWire602.Build(picked.ToWireWithSheet()),
                                ct,
                                TrackVitalsOutbound)
                            .ConfigureAwait(false);
                        await MapMonsterScopeSender.TrySendAfterJoinAsync(
                            monsterViewportTracker,
                            connection,
                            protect,
                            picked.MapId,
                            picked.PosX,
                            picked.PosY,
                            remote,
                            ct).ConfigureAwait(false);
                        var presenceJoin = GameMapPresenceRegistry.Register(
                            presenceSessionId,
                            connection,
                            protect,
                            picked.MapId,
                            picked.PosX,
                            picked.PosY,
                            picked.Angle,
                            new PlayerPresenceAppearance
                            {
                                Name10 = picked.Name10,
                                ServerClass = picked.ServerClass,
                            });
                        if (presenceJoin is not null)
                        {
                            await GameMapPresenceRegistry.NotifyJoinAsync(presenceJoin, remote, ct).ConfigureAwait(false);
                        }

                        await MoveMapOutbound.TrySendChecksumAfterJoinAsync(
                                presenceSessionId,
                                connection,
                                protect,
                                writeAsync: null,
                                ct)
                            .ConfigureAwait(false);

                        MonsterViewerRegistry.Register(
                            presenceSessionId,
                            connection,
                            protect,
                            picked.MapId,
                            picked.PosX,
                            picked.PosY,
                            monsterViewportTracker,
                            playerObjectKey: presenceJoin?.ObjectKey ?? 0,
                            clientHeroWireKey: options.JoinWireIndex,
                            currentHp: picked.CurrentHp,
                            maxHp: picked.MaxHp,
                            currentMp: picked.CurrentMp,
                            maxMp: picked.MaxMp,
                            currentShield: picked.CurrentShield,
                            maxShield: picked.MaxShield,
                            accountLogin: loggedAccountId,
                            characterName: Encoding.ASCII.GetString(picked.Name10).TrimEnd('\0'),
                            playerLevel: picked.Level,
                            experience: picked.Experience,
                            gold: (uint)Math.Clamp(picked.Zen, 0, uint.MaxValue),
                            serverClass: picked.ServerClass,
                            sheet: picked.ResolveSheet(),
                            onVitalsChanged: (hp, max) =>
                            {
                                picked.CurrentHp = hp;
                                picked.MaxHp = max;
                                Volatile.Write(ref rosterDirty, 1);
                            },
                            onShieldVitalsChanged: (sd, sdMax) =>
                            {
                                picked.CurrentShield = sd;
                                picked.MaxShield = sdMax;
                                Volatile.Write(ref rosterDirty, 1);
                            },
                            onRosterPositionChanged: (map, x, y) =>
                            {
                                picked.MapId = map;
                                picked.PosX = x;
                                picked.PosY = y;
                                Volatile.Write(ref rosterDirty, 1);
                            });

                        Console.WriteLine(
                            "[{0}] sent join map (F3 03) + inventory (F3 10 len={5}) map={1} xy=({2},{3}) name='{4}'",
                            remote,
                            joinPkt[6],
                            joinPkt[4],
                            joinPkt[5],
                            Encoding.ASCII.GetString(picked.Name10).TrimEnd('\0'),
                            invPkt.Length);
                        return;
                    }

                    if (loginLatch.IsLoggedIn
                        && sessionJoinCharacterName10 is not null)
                    {
                        var pickedGameplay = FindRosterEntry(roster, sessionJoinCharacterName10);
                        if (pickedGameplay is not null
                            && await WorldGameplayHandlers.TryHandlePacketAsync(
                                pickedGameplay,
                                monsterViewportTracker,
                                connection,
                                protect,
                                loggedAccountId,
                                sessionJoinCharacterName10,
                                presenceSessionId,
                                packet,
                                remote,
                                async (m, t) => await GamePortOutboundWire.WriteAsync(connection, protect, m, t, TrackVitalsOutbound).ConfigureAwait(false),
                                () => Volatile.Write(ref rosterDirty, 1),
                                () =>
                                {
                                    Volatile.Write(ref rosterDirty, 1);
                                    if (!string.IsNullOrEmpty(loggedAccountId))
                                    {
                                        SaveRoster(loggedAccountId, roster);
                                    }
                                },
                                ct).ConfigureAwait(false))
                        {
                            return;
                        }
                    }

                    if (loginLatch.IsLoggedIn
                        && sessionJoinCharacterName10 is not null)
                    {
                        var pickedCombat = FindRosterEntry(roster, sessionJoinCharacterName10);
                        if (pickedCombat is not null
                            && await MonsterCombatHandler.TryHandleCombatPacketAsync(
                                monsterViewportTracker,
                                connection,
                                protect,
                                pickedCombat.MapId,
                                pickedCombat.PosX,
                                pickedCombat.PosY,
                                packet,
                                remote,
                                ct,
                                pickedCombat.Level,
                                presenceSessionId,
                                pickedCombat,
                                loggedAccountId,
                                () =>
                                {
                                    Volatile.Write(ref rosterDirty, 1);
                                    MonsterViewerRegistry.TryUpdatePlayerLevel(presenceSessionId, pickedCombat.Level);
                                    if (!string.IsNullOrEmpty(loggedAccountId))
                                    {
                                        SaveRoster(loggedAccountId, roster);
                                    }
                                }).ConfigureAwait(false))
                        {
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
                                protect,
                                pickedInst.MapId,
                                instX,
                                instY,
                                remote,
                                ct).ConfigureAwait(false);
                            MonsterViewerRegistry.UpdatePosition(presenceSessionId, pickedInst.MapId, instX, instY);
                            await GameMapPresenceRegistry.BroadcastPositionAsync(
                                    presenceSessionId,
                                    pickedInst.MapId,
                                    instX,
                                    instY,
                                    remote,
                                    ct)
                                .ConfigureAwait(false);
                        }

                        return;
                    }

                    if (loginLatch.IsLoggedIn
                        && sessionJoinCharacterName10 is not null
                        && ClientWalkPackets602.TryFindWalkEndTile(packet, out var walkOff, out var walkX, out var walkY, out var walkAng, out var walkMoved))
                    {
                        var pickedWalk = FindRosterEntry(roster, sessionJoinCharacterName10);
                        if (pickedWalk is not null)
                        {
                            await PlayerWalkHandler.HandleWalkAsync(
                                    presenceSessionId,
                                    connection,
                                    protect,
                                    pickedWalk,
                                    packet[walkOff + 3],
                                    packet[walkOff + 4],
                                    walkX,
                                    walkY,
                                    walkAng,
                                    walkMoved,
                                    remote,
                                    monsterViewportTracker,
                                    ct)
                                .ConfigureAwait(false);
                            Volatile.Write(ref rosterDirty, 1);
                        }

                        return;
                    }

                    if (loginLatch.IsLoggedIn && GamePacketFinders.TryFindGameLogoutRequest(packet, out var logoutOff, out var logoutFlag))
                    {
                        var ack = new byte[] { 0xC1, 0x05, 0xF1, 0x02, logoutFlag };
                        await GamePortOutboundWire.WriteAsync(connection, protect, ack, ct).ConfigureAwait(false);
                        Console.WriteLine("[{0}] ack game logout F1 02 value=0x{1:X2} frame@{2}", remote, logoutFlag, logoutOff);
                        return;
                    }

                    if (loginLatch.IsLoggedIn && GamePacketFinders.TryFindPingResponse(packet, out var pingOff))
                    {
                        if (verbose)
                        {
                            Console.WriteLine("[{0}] ping reply 0x71 frame@{1} (no response)", remote, pingOff);
                        }

                        return;
                    }

                    // F3 00 list: only after auth. Heuristics can match byte pairs inside large C3 F1:01 login (~90 B) and
                    // must not return early — otherwise the client never receives F1 01 / character list (M6 split port).
                    var listFrameOffset = 0;
                    var listReq = loginLatch.IsLoggedIn
                        && GamePacketFinders.TryFindCharacterListRequest(packet, out listFrameOffset);

                    if (listReq)
                    {
                        var list = roster.Count > 0
                            ? await CharacterListPacket602.BuildAsync(loggedAccountId, MapRosterToWire(roster), ct).ConfigureAwait(false)
                            : CharacterListWire602.BuildEmpty();
                        LogCharacterListWire(remote, list, "F3 00");
                        await GamePortOutboundWire.WriteAsync(connection, protect, list, ct).ConfigureAwait(false);
                        return;
                    }

                    // F1 01 account login: C3 envelope + stream XOR (Android) or C1 peel — shared with LegacyLoginHost via GamePacketFinders.
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
                            TakumiStreamXorCodec.DecodeTakumiStreamXor(span, 3);
                        }
                    }

                    byte head;
                    byte sub;
                    int payloadOffset;
                    var tl = loginFrame[0];
                    if (tl == 0xC3 && loginFrame.Length >= 4)
                    {
                        head = loginFrame[2];
                        sub = loginFrame[3];
                        payloadOffset = 4;
                    }
                    else if (tl == 0xC1 && loginFrame.Length >= 4)
                    {
                        head = loginFrame[2];
                        sub = loginFrame[3];
                        payloadOffset = 4;
                    }
                    else
                    {
                        return;
                    }

                    if (head != 0xF1 || sub != 0x01 || loginFrame.Length < 59)
                    {
                        if (loginLatch.IsLoggedIn && loginFrame.Length <= 48 && verbose)
                        {
                            Console.WriteLine(
                                "[{0}] not login pkt — hex={1} head=0x{2:X2} sub=0x{3:X2}",
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
                        Console.WriteLine("[{0}] login rejected: version mismatch", remote);
                        await WriteLoginResultAsync(connection, protect, 0x06, ct).ConfigureAwait(false);
                        return;
                    }

                    if (!clientSer.SequenceEqual(serverSerial))
                    {
                        Console.WriteLine("[{0}] login rejected: serial mismatch", remote);
                        await WriteLoginResultAsync(connection, protect, 0x06, ct).ConfigureAwait(false);
                        return;
                    }

                    if (!await AccountCredentialGate.TryValidateLoginAsync(id, pass, accounts, ct).ConfigureAwait(false))
                    {
                        if (verbose)
                        {
                            Console.WriteLine(
                                "[{0}] login rejected: bad credentials '{1}' passLen={2} passPreview={3}",
                                remote,
                                id,
                                pass.Length,
                                pass.Length <= 12 ? pass : pass[..12] + "…");
                        }
                        else
                        {
                            Console.WriteLine("[{0}] login rejected: bad credentials '{1}'", remote, id);
                        }

                        await WriteLoginResultAsync(connection, protect, 0x00, ct).ConfigureAwait(false);
                        return;
                    }

                    if (options.RequireSignedSessionTicketWire)
                    {
                        if (wireVerifiedAccountNorm is null)
                        {
                            Console.WriteLine(
                                "[{0}] login rejected: F1 A6 signed attach required before F1 01 (TAKUMI_GAME_TICKET_WIRE)",
                                remote);
                            await WriteLoginResultAsync(connection, protect, 0x00, ct).ConfigureAwait(false);
                            return;
                        }

                        if (!string.Equals(
                                CharacterRosterMerge.NormaliseName(id),
                                wireVerifiedAccountNorm,
                                StringComparison.Ordinal))
                        {
                            Console.WriteLine(
                                "[{0}] login rejected: F1 01 id does not match verified wire attach account",
                                remote);
                            await WriteLoginResultAsync(connection, protect, 0x00, ct).ConfigureAwait(false);
                            return;
                        }
                    }

                    if (options.RequireLoginPostgresHandoff && !options.RequireSignedSessionTicketWire)
                    {
                        if (TakumiPostgresMirror.SessionHandoff is not { } sh)
                        {
                            Console.WriteLine(
                                "[{0}] login rejected: TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF but session DB not enabled (set TAKUMI_SESSION_HANDOFF_DB=1 + PG connection)",
                                remote);
                            await WriteLoginResultAsync(connection, protect, 0x00, ct).ConfigureAwait(false);
                            return;
                        }

                        var clientIp = ConnectClientIp.TryFormatIp(tcp.Client.RemoteEndPoint);
                        bool consumed;
                        try
                        {
                            consumed = await sh.TryConsumePendingAsync(id, clientIp, options.LoginHandoffMatchClientIp, ct).ConfigureAwait(false);
                        }
                        catch (Exception ex) when (IsPostgresException(ex))
                        {
                            Console.WriteLine(
                                "[{0}] login rejected: session handoff DB error ({1})",
                                remote,
                                ex.Message);
                            await WriteLoginResultAsync(connection, protect, 0x00, ct).ConfigureAwait(false);
                            return;
                        }

                        if (!consumed)
                        {
                            Console.WriteLine(
                                "[{0}] login rejected: no consumable session_ticket for account '{1}' (login on legacy port first, or disable TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF)",
                                remote,
                                id);
                            await WriteLoginResultAsync(connection, protect, 0x00, ct).ConfigureAwait(false);
                            return;
                        }
                    }

                    loginLatch.SetLoggedIn();
                    loggedAccountId = id;
                    await CharacterRosterHostLoad.LoadOnLoginAsync(id, roster, rosterDbMergeOverlay, ct).ConfigureAwait(false);

                    Console.WriteLine("[{0}] login ok id={1} rosterCount={2}", remote, id, roster.Count);
                    await WriteLoginResultAsync(connection, protect, 0x01, ct).ConfigureAwait(false);

                    if (!options.SkipAutoCharacterList)
                    {
                        var list = roster.Count > 0
                            ? await CharacterListPacket602.BuildAsync(loggedAccountId, MapRosterToWire(roster), ct).ConfigureAwait(false)
                            : CharacterListWire602.BuildEmpty();
                        LogCharacterListWire(remote, list, "after login (auto)");
                        await GamePortOutboundWire.WriteAsync(connection, protect, list, ct).ConfigureAwait(false);
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
        }
        catch (Exception ex)
        {
            Console.WriteLine("[{0}] game minimal session error: {1}", remote, ex.Message);
        }
        finally
        {
            if (protectInboundPumpCts is not null)
            {
                try
                {
                    protectInboundPumpCts.Cancel();
                }
                catch
                {
                }
            }

            if (protectInboundPumpTask is not null)
            {
                try
                {
                    await protectInboundPumpTask.ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    Console.Error.WriteLine("[{0}] protect inbound pump fault: {1}", remote, ex);
                }
            }

            protectInboundPumpCts?.Dispose();

            if (!string.IsNullOrEmpty(loggedAccountId) && roster.Count > 0 && Volatile.Read(ref rosterDirty) != 0)
            {
                try
                {
                    SaveRoster(loggedAccountId, roster);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("[game-roster] disconnect flush failed: {0}", ex.Message);
                }
            }

            CharacterRosterMirrorWriter.TryDrainPendingUpserts(TimeSpan.FromMilliseconds(900));
            PlayerShopSession.FlushInventoryMirrorOnDisconnect(loggedAccountId, sessionJoinCharacterName10, presenceSessionId);
            PlayerWarehouseSession.FlushOnDisconnect(loggedAccountId, presenceSessionId);
            PlayerTradeSession.Close(presenceSessionId);
            PlayerUiSession.Clear(presenceSessionId);
            InventorySlotMirrorWriter.TryDrainPendingOps(TimeSpan.FromMilliseconds(900));
            WarehouseSlotMirrorWriter.TryDrainPendingOps(TimeSpan.FromMilliseconds(900));

            await GameMapPresenceRegistry.UnregisterAsync(presenceSessionId, ct).ConfigureAwait(false);
            MonsterViewerRegistry.Unregister(presenceSessionId);
            MoveMapSessionState.Remove(presenceSessionId);

            try
            {
                tcp.Dispose();
            }
            catch
            {
                // ignore
            }
        }
    }

    static void SaveRoster(string accountId, IReadOnlyList<GameRosterEntry> roster)
    {
        var root = new RosterSaveRoot();
        foreach (var e in roster)
        {
            var name = Encoding.ASCII.GetString(e.Name10).TrimEnd('\0', ' ');
            if (string.IsNullOrEmpty(name))
            {
                continue;
            }

            root.Characters.Add(
                new RosterSaveChar
                {
                    Name = name,
                    ServerClass = e.ServerClass,
                    Level = e.Level,
                    Experience = e.Experience,
                    MapId = e.MapId,
                    PosX = e.PosX,
                    PosY = e.PosY,
                    Angle = e.Angle,
                    CurrentHp = e.CurrentHp,
                    MaxHp = e.MaxHp,
                    CurrentMp = e.CurrentMp,
                    MaxMp = e.MaxMp,
                    Zen = e.Zen,
                    CurrentShield = e.CurrentShield,
                    MaxShield = e.MaxShield,
                    Strength = e.Strength,
                    Dexterity = e.Dexterity,
                    Vitality = e.Vitality,
                    Energy = e.Energy,
                    Leadership = e.Leadership,
                    LevelUpPoint = e.LevelUpPoint,
                    CurrentBp = e.CurrentBp,
                    MaxBp = e.MaxBp,
                });
        }

        if (!CharacterRosterBootstrap.ShouldSkipJsonExportOnSave())
        {
            var path = GameRosterDisk.GetRosterFilePath(accountId);
            var dir = Path.GetDirectoryName(path);
            if (!string.IsNullOrEmpty(dir))
            {
                Directory.CreateDirectory(dir);
            }

            var json = JsonSerializer.Serialize(root, GameRosterDisk.JsonOptions);
            lock (GameRosterDisk.JsonFileLock)
            {
                File.WriteAllText(path, json);
            }
        }

        CharacterRosterMirrorWriter.ScheduleReplaceAccountRoster(accountId, BuildCharacterRosterRows(roster));
    }

    static CharacterRosterRow[] BuildCharacterRosterRows(IReadOnlyList<GameRosterEntry> roster)
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
                    e.Zen,
                    e.CurrentShield,
                    e.MaxShield,
                    e.Strength,
                    e.Dexterity,
                    e.Vitality,
                    e.Energy,
                    e.Leadership,
                    e.LevelUpPoint,
                    e.CurrentBp,
                    e.MaxBp,
                    e.Experience));
        }

        return list.ToArray();
    }

    sealed class RosterSaveRoot
    {
        public List<RosterSaveChar> Characters { get; set; } = new();
    }

    sealed class RosterSaveChar
    {
        public string Name { get; set; } = "";
        public byte ServerClass { get; set; }
        public ushort Level { get; set; }

        public uint Experience { get; set; }

        public byte MapId { get; set; }
        public byte PosX { get; set; }
        public byte PosY { get; set; }
        public byte Angle { get; set; }

        public int CurrentHp { get; set; }

        public int MaxHp { get; set; }

        public int CurrentMp { get; set; }

        public int MaxMp { get; set; }

        public long Zen { get; set; }

        public int CurrentShield { get; set; }

        public int MaxShield { get; set; }

        public ushort Strength { get; set; }

        public ushort Dexterity { get; set; }

        public ushort Vitality { get; set; }

        public ushort Energy { get; set; }

        public ushort Leadership { get; set; }

        public ushort LevelUpPoint { get; set; }

        public int CurrentBp { get; set; }

        public int MaxBp { get; set; }
    }

    static GameRosterEntry? FindRosterEntry(List<GameRosterEntry> roster, byte[] joinName10)
    {
        foreach (var e in roster)
        {
            if (GameNameUtil.NameBytesEqual(e.Name10, joinName10))
            {
                return e;
            }
        }

        return null;
    }

    static List<CharacterRosterWire> MapRosterToWire(List<GameRosterEntry> roster)
    {
        var list = new List<CharacterRosterWire>(roster.Count);
        foreach (var e in roster)
        {
            list.Add(e.ToWireWithSheet());
        }

        return list;
    }

    static void LogCharacterListWire(string remote, byte[] list, string tag)
    {
        if (list.Length < 4)
        {
            Console.Error.WriteLine("[wire] {0} F3 00 {1}: len={2}", remote, tag, list.Length);
            return;
        }

        var previewLen = Math.Min(24, list.Length);
        Console.Error.WriteLine(
            "[wire] {0} F3 00 {1}: totalTcp={2} c1Len=0x{3:X2} preview={4}",
            remote,
            tag,
            list.Length,
            list[1],
            Convert.ToHexString(list.AsSpan(0, previewLen)));
    }

    static void BuxXor(byte[] buf)
    {
        ReadOnlySpan<byte> xorTable = stackalloc byte[] { 0xFC, 0xCF, 0xAB };
        for (var i = 0; i < buf.Length; i++)
        {
            buf[i] ^= xorTable[i % 3];
        }
    }

    static Task WriteLoginResultAsync(
        Connection connection,
        (byte EncDecKey1, byte EncDecKey2)? clientProtectOutbound,
        byte result,
        CancellationToken ct)
    {
        var pkt = LoginAccountWire602.BuildLoginResult(result);
        return GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, pkt, ct);
    }

    static bool IsPostgresException(Exception ex)
    {
        for (var e = ex; e is not null; e = e.InnerException)
        {
            var name = e.GetType().FullName;
            if (name is not null && name.StartsWith("Npgsql.", StringComparison.Ordinal))
            {
                return true;
            }
        }

        return false;
    }

    sealed class LoginLatch
    {
        private int _loggedIn;

        public bool IsLoggedIn => Volatile.Read(ref this._loggedIn) != 0;

        public void SetLoggedIn() => Volatile.Write(ref this._loggedIn, 1);
    }
}
