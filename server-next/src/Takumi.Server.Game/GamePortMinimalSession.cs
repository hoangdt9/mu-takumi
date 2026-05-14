using System.Buffers;
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
        var accounts = options.AuthAccounts!;
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
        var loginLatch = new LoginLatch();

        try
        {
            tcp.NoDelay = true;
            ConnectTcpKeepAlive.TryApply(tcp.Client);
            var socketConnection = SocketConnection.Create(tcp.Client);
            using var connection = new Connection(
                socketConnection,
                new PipelinedDecryptor(socketConnection.Input, options.ServerDecryptKeys, DefaultKeys.Xor32Key),
                encryptionPipe: null,
                new NullLogger<Connection>());

            var join = LoginAccountWire602.BuildJoinPacket(result: 1, options.JoinWireIndex, joinVersion);
            await connection.Output.WriteAsync(join, ct).ConfigureAwait(false);
            await connection.Output.FlushAsync(ct).ConfigureAwait(false);
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

            try
            {
                await connection.BeginReceiveAsync().ConfigureAwait(false);
            }
            finally
            {
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
                        await connection.Output.WriteAsync(resp, ct).ConfigureAwait(false);
                        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
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

                    if (GamePacketFinders.TryFindDeleteCharacterRequest(packet, out var deleteOff, out var deleteName10, out _))
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
                                "[{0}] delete character — not in roster name='{1}' frame@{2}",
                                remote,
                                Encoding.ASCII.GetString(deleteName10).TrimEnd('\0'),
                                deleteOff);
                            await connection.Output.WriteAsync(CharacterCreateWire602.BuildDeleteResponse(2), ct).ConfigureAwait(false);
                            await connection.Output.FlushAsync(ct).ConfigureAwait(false);
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

                        var spawn = new JoinMapSpawnWire(picked.MapId, picked.PosX, picked.PosY, picked.Angle);
                        var joinPkt = JoinMapServerWire602.Build(ToWire(picked), spawn);
                        var invPkt = await JoinInventoryPacket602.BuildAsync(TakumiPostgresMirror.InventorySlots, loggedAccountId, joinName10, ct).ConfigureAwait(false);
                        await connection.Output.WriteAsync(joinPkt, ct).ConfigureAwait(false);
                        await connection.Output.WriteAsync(invPkt, ct).ConfigureAwait(false);
                        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                        sessionJoinCharacterName10 = new byte[10];
                        Buffer.BlockCopy(joinName10, 0, sessionJoinCharacterName10, 0, 10);
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
                        && sessionJoinCharacterName10 is not null
                        && GamePacketFinders.TryFindMoveMapRequest(packet, out var moveOff, out _, out var mapIdx))
                    {
                        var pickedMove = FindRosterEntry(roster, sessionJoinCharacterName10);
                        var ackMove = new byte[] { 0xC1, 0x05, 0x8E, 0x03, 0x01 };
                        await connection.Output.WriteAsync(ackMove, ct).ConfigureAwait(false);
                        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                        if (pickedMove is not null)
                        {
                            pickedMove.MapId = (byte)(mapIdx & 0xFF);
                            SaveRoster(loggedAccountId!, roster);
                            var mvSpawn = new JoinMapSpawnWire(pickedMove.MapId, pickedMove.PosX, pickedMove.PosY, pickedMove.Angle);
                            var joinPktMove = JoinMapServerWire602.Build(ToWire(pickedMove), mvSpawn);
                            var invMove = await JoinInventoryPacket602.BuildAsync(TakumiPostgresMirror.InventorySlots, loggedAccountId, sessionJoinCharacterName10, ct).ConfigureAwait(false);
                            await connection.Output.WriteAsync(joinPktMove, ct).ConfigureAwait(false);
                            await connection.Output.WriteAsync(invMove, ct).ConfigureAwait(false);
                            await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                            Console.WriteLine("[{0}] move map + F3 03 + F3 10 len={2} mapId={1} frame@{3}", remote, mapIdx, invMove.Length, moveOff);
                        }

                        return;
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
                            }

                            pickedWalk.Angle = walkAng;
                            Volatile.Write(ref rosterDirty, 1);
                        }

                        return;
                    }

                    if (loginLatch.IsLoggedIn && GamePacketFinders.TryFindGameLogoutRequest(packet, out var logoutOff, out var logoutFlag))
                    {
                        var ack = new byte[] { 0xC1, 0x05, 0xF1, 0x02, logoutFlag };
                        await connection.Output.WriteAsync(ack, ct).ConfigureAwait(false);
                        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                        Console.WriteLine("[{0}] ack game logout F1 02 value=0x{1:X2} frame@{2}", remote, logoutFlag, logoutOff);
                        return;
                    }

                    var listReq = GamePacketFinders.TryFindCharacterListRequest(packet, out var listFrameOffset);
                    if (!listReq
                        && loginLatch.IsLoggedIn
                        && packet.Length == 12
                        && packet[0] == 0xC3)
                    {
                        listReq = true;
                        listFrameOffset = 0;
                    }

                    if (listReq)
                    {
                        if (!loginLatch.IsLoggedIn)
                        {
                            Console.WriteLine("[{0}] F3 00 before login — ignored", remote);
                            return;
                        }

                        var list = roster.Count > 0 ? CharacterListWire602.Build(MapRosterToWire(roster)) : CharacterListWire602.BuildEmpty();
                        LogCharacterListWire(remote, list, "F3 00");
                        await connection.Output.WriteAsync(list, ct).ConfigureAwait(false);
                        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
                        return;
                    }

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

                    if (head != 0xF1 || sub != 0x01 || packet.Length < 59)
                    {
                        if (loginLatch.IsLoggedIn && packet.Length <= 48 && verbose)
                        {
                            Console.WriteLine(
                                "[{0}] not login pkt — hex={1} head=0x{2:X2} sub=0x{3:X2}",
                                remote,
                                Convert.ToHexString(packet),
                                head,
                                sub);
                        }

                        return;
                    }

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
                        Console.WriteLine("[{0}] login rejected: version mismatch", remote);
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
                        Console.WriteLine("[{0}] login rejected: bad credentials '{1}'", remote, id);
                        await WriteLoginResultAsync(connection, 0x00, ct).ConfigureAwait(false);
                        return;
                    }

                    if (options.RequireSignedSessionTicketWire)
                    {
                        if (wireVerifiedAccountNorm is null)
                        {
                            Console.WriteLine(
                                "[{0}] login rejected: F1 A6 signed attach required before F1 01 (TAKUMI_GAME_TICKET_WIRE)",
                                remote);
                            await WriteLoginResultAsync(connection, 0x00, ct).ConfigureAwait(false);
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
                            await WriteLoginResultAsync(connection, 0x00, ct).ConfigureAwait(false);
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
                            await WriteLoginResultAsync(connection, 0x00, ct).ConfigureAwait(false);
                            return;
                        }

                        var clientIp = ConnectClientIp.TryFormatIp(tcp.Client.RemoteEndPoint);
                        var consumed = await sh.TryConsumePendingAsync(id, clientIp, options.LoginHandoffMatchClientIp, ct).ConfigureAwait(false);
                        if (!consumed)
                        {
                            Console.WriteLine(
                                "[{0}] login rejected: no consumable session_ticket for account '{1}' (login on legacy port first, or disable TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF)",
                                remote,
                                id);
                            await WriteLoginResultAsync(connection, 0x00, ct).ConfigureAwait(false);
                            return;
                        }
                    }

                    loginLatch.SetLoggedIn();
                    loggedAccountId = id;
                    roster.Clear();
                    roster.AddRange(GameRosterDisk.LoadEntries(id));
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
                                });
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("[roster-db] merge after login failed for {0}: {1}", id, ex.Message);
                        }
                    }

                    Console.WriteLine("[{0}] login ok id={1} rosterCount={2}", remote, id, roster.Count);
                    await WriteLoginResultAsync(connection, 0x01, ct).ConfigureAwait(false);

                    if (!options.SkipAutoCharacterList)
                    {
                        var list = roster.Count > 0 ? CharacterListWire602.Build(MapRosterToWire(roster)) : CharacterListWire602.BuildEmpty();
                        LogCharacterListWire(remote, list, "after login (auto)");
                        await connection.Output.WriteAsync(list, ct).ConfigureAwait(false);
                        await connection.Output.FlushAsync(ct).ConfigureAwait(false);
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
            if (!string.IsNullOrEmpty(loggedAccountId) && roster.Count > 0)
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
                    MapId = e.MapId,
                    PosX = e.PosX,
                    PosY = e.PosY,
                    Angle = e.Angle,
                });
        }

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
                new CharacterRosterRow
                {
                    Name = name,
                    ServerClass = e.ServerClass,
                    Level = e.Level,
                    MapId = e.MapId,
                    PosX = e.PosX,
                    PosY = e.PosY,
                    Angle = e.Angle,
                });
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
        public byte MapId { get; set; }
        public byte PosX { get; set; }
        public byte PosY { get; set; }
        public byte Angle { get; set; }
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

    static CharacterRosterWire ToWire(GameRosterEntry e) => new(e.Name10, e.ServerClass, e.Level);

    static List<CharacterRosterWire> MapRosterToWire(List<GameRosterEntry> roster)
    {
        var list = new List<CharacterRosterWire>(roster.Count);
        foreach (var e in roster)
        {
            list.Add(ToWire(e));
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

    sealed class LoginLatch
    {
        private int _loggedIn;

        public bool IsLoggedIn => Volatile.Read(ref this._loggedIn) != 0;

        public void SetLoggedIn() => Volatile.Write(ref this._loggedIn, 1);
    }
}
