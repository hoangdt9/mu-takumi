using System.Buffers;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using Microsoft.Extensions.Logging.Abstractions;
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
    public IReadOnlyDictionary<string, string>? AuthAccounts { get; init; }

    public byte[]? AuthServerSerial16 { get; init; }

    public bool SkipAutoCharacterList { get; init; }

    /// <summary>When true, successful F1 01 requires a pending <c>session_ticket</c> row (see <c>TakumiPostgresMirror.SessionHandoff</c>).</summary>
    public bool RequireLoginPostgresHandoff { get; init; }

    /// <summary>When <see cref="RequireLoginPostgresHandoff"/> is true, match <c>client_ip</c> stored at login (set <c>TAKUMI_GAME_HANDOFF_MATCH_IP=0</c> to disable).</summary>
    public bool LoginHandoffMatchClientIp { get; init; } = true;

    /// <summary>When true, client must send <c>F1 0xA6</c> signed attach before <c>F1 01</c>; ticket is consumed on attach (not IP-only consume).</summary>
    public bool RequireSignedSessionTicketWire { get; init; }
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

            var join = LoginAccountWire602.BuildJoinPacket(result: 1, options.JoinWireIndex, options.JoinVersion5);
            await connection.Output.WriteAsync(join, ct).ConfigureAwait(false);
            await connection.Output.FlushAsync(ct).ConfigureAwait(false);
            var msg = $"[{remote}] sent join C1 F1 00 ({join.Length} bytes) index={options.JoinWireIndex} (bootstrap-only)";
            Console.WriteLine(msg);
            Console.Error.WriteLine(msg);

            connection.PacketReceived += OnPacketAsync;

            try
            {
                await connection.BeginReceiveAsync().ConfigureAwait(false);
            }
            finally
            {
                connection.PacketReceived -= OnPacketAsync;
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
