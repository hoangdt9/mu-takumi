using System.Buffers;
using System.IO.Pipelines;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using Microsoft.Extensions.Logging;
using MUnique.OpenMU.Network;
using MUnique.OpenMU.Network.SimpleModulus;
using MUnique.OpenMU.Network.Xor;
using Pipelines.Sockets.Unofficial;
using Takumi.Server.Connect;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

public sealed class GamePortListenOptions
{
    public required SimpleModulusKeys ServerDecryptKeys { get; init; }

    public required byte[] JoinVersion5 { get; init; }

    public ushort JoinWireIndex { get; init; }

    public bool Verbose { get; init; }

    public bool ReuseAddress { get; init; }

    /// <summary>When set with <see cref="AuthServerSerial16"/>, enables login + roster + join on this TCP (split <c>TAKUMI_GAME_PORT</c>).</summary>
    public Dictionary<string, string>? AuthAccounts { get; init; }

    public byte[]? AuthServerSerial16 { get; init; }

    public bool SkipAutoCharacterList { get; init; }

    /// <summary>When true, successful F1 01 requires a pending <c>session_ticket</c> row (see <c>TakumiPostgresMirror.SessionHandoff</c>).</summary>
    public bool RequireLoginPostgresHandoff { get; init; }

    /// <summary>When <see cref="RequireLoginPostgresHandoff"/> is true, match <c>client_ip</c> stored at login (set <c>TAKUMI_GAME_HANDOFF_MATCH_IP=0</c> to disable).</summary>
    public bool LoginHandoffMatchClientIp { get; init; } = true;

    /// <summary>When true, client must send <c>F1 0xA6</c> signed attach before <c>F1 01</c>; ticket is consumed on attach (not IP-only consume).</summary>
    public bool RequireSignedSessionTicketWire { get; init; }

    /// <summary>
    /// When set, outbound MU frames are obfuscated with <see cref="TakumiClientProtectWire602.EncryptInPlace"/> so Android
    /// <c>gProtect.DecryptData</c> on GS TCP (55901+) restores plain packets. Inbound: Android <c>sSend</c> runs
    /// <c>gProtect.EncryptData</c> on the entire buffer after <c>SendPacket</c> SimpleModulus — use
    /// <see cref="TakumiClientProtectInboundPump"/> before <c>PipelinedDecryptor</c> (see <c>GamePortMinimalSession</c>).
    /// </summary>
    public (byte EncDecKey1, byte EncDecKey2)? ClientProtectOutboundKeys { get; init; }
}

/// <summary>Join-only bootstrap (no login).</summary>
public static class GamePortBootstrapSession
{
    public static async Task RunAsync(
        TcpClient tcp,
        string remote,
        GamePortListenOptions options,
        CancellationToken ct)
    {
        Task? protectInboundPumpTask = null;
        CancellationTokenSource? protectInboundPumpCts = null;
        try
        {
            Console.WriteLine(
                "[{0}] m6_bootstrap_session_begin hasClientProtectKeys={1} rxBuild=M6-2026-05-15c",
                remote,
                options.ClientProtectOutboundKeys.HasValue);
            tcp.NoDelay = true;
            ConnectTcpKeepAlive.TryApply(tcp.Client);
            var socketConnection = SocketConnection.Create(tcp.Client);
            PipeReader pipelinedDecryptInput = socketConnection.Input;
            if (options.ClientProtectOutboundKeys is { } wireProtectKeys)
            {
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
                b.SetMinimumLevel(options.Verbose ? LogLevel.Information : LogLevel.Warning);
            });
            var connectionLogger = connLogFactory.CreateLogger<Connection>();

            using var connection = new Connection(
                socketConnection,
                new PipelinedDecryptor(pipelinedDecryptInput, options.ServerDecryptKeys, DefaultKeys.Xor32Key),
                encryptionPipe: null,
                connectionLogger);

            var join = LoginAccountWire602.BuildJoinPacket(result: 1, options.JoinWireIndex, options.JoinVersion5);
            await GamePortOutboundWire.WriteAsync(connection, options.ClientProtectOutboundKeys, join, ct).ConfigureAwait(false);
            var msg = $"[{remote}] sent join C1 F1 00 ({join.Length} bytes) index={options.JoinWireIndex} (bootstrap-only)";
            Console.WriteLine(msg);
            Console.Error.WriteLine(msg);

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
                var packet = packetSeq.ToArray();
                if (packet.Length == 0)
                {
                    return;
                }

                if (options.Verbose)
                {
                    Console.WriteLine(
                        "[{0}] decrypted len={1} head=0x{2:X2} hex={3}",
                        remote,
                        packet.Length,
                        packet[0],
                        Convert.ToHexString(packet));
                }
            }
        }
        catch (OperationCanceledException) when (ct.IsCancellationRequested)
        {
        }
        catch (Exception ex)
        {
            Console.WriteLine("[{0}] game bootstrap error: {1}", remote, ex.Message);
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
}
