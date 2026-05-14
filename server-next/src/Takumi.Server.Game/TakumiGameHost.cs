using System.Buffers;
using System.Net;
using System.Net.Sockets;
using Microsoft.Extensions.Logging.Abstractions;
using MUnique.OpenMU.Network;
using Pipelines.Sockets.Unofficial;
using MUnique.OpenMU.Network.SimpleModulus;
using MUnique.OpenMU.Network.Xor;
using Takumi.Server.Game.Networking;

namespace Takumi.Server.Game;

/// <summary>Minimal TCP game listener: accept → SimpleModulus+Xor32 decrypt → send C1 F1 00 join → receive loop (M6 bootstrap).</summary>
public static class TakumiGameHost
{
    public static async Task RunAsync(
        GameHostOptions options,
        SimpleModulusKeys serverDecryptKeys,
        CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(options.JoinVersion5);
        if (options.JoinVersion5.Length != 5)
        {
            throw new ArgumentException("JoinVersion5 must be exactly 5 bytes.", nameof(options));
        }

        TcpListener listener;
        try
        {
            listener = new TcpListener(IPAddress.Any, options.Port);
            listener.Server.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, options.ReuseAddress);
            listener.Start(options.TcpBacklog);
        }
        catch (SocketException ex) when (ex.SocketErrorCode == SocketError.AddressAlreadyInUse)
        {
            Console.Error.WriteLine(
                "Cannot bind game port {0}: address already in use. Stop the other listener or set TAKUMI_GAME_PORT.\n" +
                "  Check: lsof -nP -iTCP:{0} -sTCP:LISTEN",
                options.Port);
            throw;
        }

        Console.WriteLine(
            "Takumi.Server.Game listening on *:{0} (join C1 F1 00, decrypt=SimpleModulus+Xor32). Ctrl+C to stop.\n" +
            "  Join version (5 B wire): {1}\n" +
            "  Keepalive: TAKUMI_GAME_KEEPALIVE_SECONDS (effective {2}; 0=off)",
            options.Port,
            Convert.ToHexString(options.JoinVersion5),
            options.KeepAliveInterval <= TimeSpan.Zero ? "off" : options.KeepAliveInterval.TotalSeconds.ToString());

        try
        {
            while (!cancellationToken.IsCancellationRequested)
            {
                TcpClient tcp;
                try
                {
                    tcp = await listener.AcceptTcpClientAsync(cancellationToken).ConfigureAwait(false);
                }
                catch (ObjectDisposedException)
                {
                    break;
                }
                catch (SocketException)
                {
                    break;
                }
                catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested)
                {
                    break;
                }

                var acceptedFrom = tcp.Client.RemoteEndPoint?.ToString() ?? "?";
                var acceptMsg = $"[game] event=tcp_accept remote={acceptedFrom} port={options.Port}";
                Console.WriteLine(acceptMsg);
                Console.Error.WriteLine(acceptMsg);

                _ = HandleGameClientAsync(tcp, options, serverDecryptKeys, cancellationToken);
            }
        }
        catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested)
        {
        }
        finally
        {
            try
            {
                listener.Stop();
            }
            catch
            {
                // ignore double-stop / races during shutdown
            }
        }
    }

    private static async Task HandleGameClientAsync(
        TcpClient tcp,
        GameHostOptions options,
        SimpleModulusKeys serverDecryptKeys,
        CancellationToken cancellationToken)
    {
        var remote = tcp.Client.RemoteEndPoint?.ToString() ?? "?";
        try
        {
            using (tcp)
            {
                tcp.NoDelay = true;
                var socket = tcp.Client;
                SocketIdleHelpers.TryApplyGamePortTcpKeepAlive(socket);
                var socketConnection = SocketConnection.Create(socket);
                using var connection = new Connection(
                    socketConnection,
                    new PipelinedDecryptor(socketConnection.Input, serverDecryptKeys, DefaultKeys.Xor32Key),
                    encryptionPipe: null,
                    new NullLogger<Connection>());

                var join = GameJoinWire.BuildJoinPacket(options.JoinResult, options.JoinIndex, options.JoinVersion5);
                await connection.Output.WriteAsync(join, cancellationToken).ConfigureAwait(false);
                await connection.Output.FlushAsync(cancellationToken).ConfigureAwait(false);

                var joinMsg =
                    $"[game] event=join_sent remote={remote} len={join.Length} head=C1 F1 00 (client should recv tcp first byte 0xC1)";
                Console.WriteLine(joinMsg);
                Console.Error.WriteLine(joinMsg);

                using var writeGate = new SemaphoreSlim(1, 1);
                using var connectionCts = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
                Task? keepAliveTask = null;
                if (options.KeepAliveInterval > TimeSpan.Zero)
                {
                    keepAliveTask = RunStandaloneGameKeepAliveAsync(
                        connection,
                        writeGate,
                        remote,
                        options.Verbose,
                        options.KeepAliveInterval,
                        connectionCts.Token);
                }

                async ValueTask OnPacketAsync(ReadOnlySequence<byte> packetSeq)
                {
                    var packet = packetSeq.ToArray();
                    LogDecryptedRx(remote, packet, options.Verbose);
                    await Task.CompletedTask.ConfigureAwait(false);
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
                }
            }
        }
        catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested)
        {
        }
        catch (Exception ex)
        {
            Console.WriteLine("[game] event=client_error remote={0} message={1}", remote, ex.Message);
        }
    }

    private static void LogDecryptedRx(string remote, ReadOnlyMemory<byte> packet, bool verbose)
    {
        if (packet.Length == 0)
        {
            return;
        }

        var span = packet.Span;
        var head = span[0];
        var c1Code = span.Length >= 3 ? span[2] : (byte)0;
        var c1Sub = span.Length >= 4 ? span[3] : (byte)0;

        Console.WriteLine(
            "[game] event=decrypted_rx remote={0} len={1} head=0x{2:X2} c1_code=0x{3:X2} c1_sub=0x{4:X2}",
            remote,
            packet.Length,
            head,
            c1Code,
            c1Sub);

        if (verbose)
        {
            Console.WriteLine("[game] event=decrypted_rx_hex remote={0} hex={1}", remote, Convert.ToHexString(span));
        }
    }

    private static async Task RunStandaloneGameKeepAliveAsync(
        Connection connection,
        SemaphoreSlim writeGate,
        string remote,
        bool verbose,
        TimeSpan interval,
        CancellationToken cancellationToken)
    {
        try
        {
            while (!cancellationToken.IsCancellationRequested)
            {
                await Task.Delay(interval, cancellationToken).ConfigureAwait(false);
                await writeGate.WaitAsync(cancellationToken).ConfigureAwait(false);
                try
                {
                    await connection.Output.WriteAsync(GamePortKeepAliveWire.PingRequest, cancellationToken).ConfigureAwait(false);
                    await connection.Output.FlushAsync(cancellationToken).ConfigureAwait(false);
                    if (verbose)
                    {
                        Console.WriteLine("[{0}] keepalive sent C1 03 71", remote);
                    }
                }
                finally
                {
                    writeGate.Release();
                }
            }
        }
        catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested)
        {
        }
        catch (Exception ex)
        {
            Console.WriteLine("[{0}] keepalive task error: {1}", remote, ex.Message);
        }
    }
}
