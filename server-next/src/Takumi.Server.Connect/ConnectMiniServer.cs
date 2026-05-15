using System.Net.Sockets;
using Takumi.Server.Protocol;

namespace Takumi.Server.Connect;
public sealed class ConnectMiniServerOptions
{
    public required string PublicHost { get; init; }

    public required ushort GamePort { get; init; }

    public bool Verbose { get; init; }

    /// <summary>Wire bytes for <c>C2 F4 06</c> server list (from <see cref="ConnectServerList602"/>).</summary>
    public required byte[] ServerList602 { get; init; }

    /// <summary>When true, send <see cref="ServerList602"/> immediately after TCP accept (before reading client).</summary>
    /// <remarks>
    /// Some LAN / Docker paths delay or drop the first client→server <c>C1 F4 06</c>; the client recv thread then
    /// blocks forever with no <c>C2 F4 06</c>. Pushing the list on accept matches many MU clients that only parse
    /// inbound data. Set <c>TAKUMI_CONNECT_SEND_LIST_ON_ACCEPT=0</c> to restore request-only behavior.
    /// </remarks>
    public bool SendServerListOnAccept { get; init; } = true;

    /// <summary>When true, list requests receive <see cref="ConnectServerBusy602"/> instead of the list (QA).</summary>
    public bool ReturnBusy { get; init; }

    public byte BusyServerIndex { get; init; }
}

/// <summary>Accept loop + per-client handler for the standalone connect TCP port.</summary>
public static class ConnectMiniServer
{
    public static async Task RunAcceptLoopAsync(
        TcpListener listener,
        ConnectMiniServerOptions options,
        CancellationToken ct)
    {
        try
        {
            while (!ct.IsCancellationRequested)
            {
                TcpClient tcp;
                try
                {
                    tcp = await listener.AcceptTcpClientAsync(ct).ConfigureAwait(false);
                }
                catch (ObjectDisposedException)
                {
                    break;
                }
                catch (SocketException)
                {
                    break;
                }
                catch (OperationCanceledException) when (ct.IsCancellationRequested)
                {
                    break;
                }

                _ = HandleClientAsync(tcp, options, ct);
            }
        }
        catch (OperationCanceledException) when (ct.IsCancellationRequested)
        {
            // shutdown
        }
    }

    public static async Task HandleClientAsync(TcpClient tcp, ConnectMiniServerOptions options, CancellationToken ct)
    {
        var remote = tcp.Client.RemoteEndPoint?.ToString() ?? "?";
        var acceptMsg =
            $"[connect] TCP accept from {remote} (Connect Server — waiting for patch / F4 02|06 / F4 03)";
        Console.WriteLine(acceptMsg);
        Console.Error.WriteLine(acceptMsg);
        Console.Out.Flush();
        Console.Error.Flush();
        try
        {
            tcp.NoDelay = true;
            ConnectTcpKeepAlive.TryApply(tcp.Client);
            await using var stream = tcp.GetStream();
            var sentListOnAccept = false;
            if (options.SendServerListOnAccept && !options.ReturnBusy)
            {
                // Plain synchronous NetworkStream write (no raw Socket.Send before GetStream — mixing can
                // confuse some stacks; no WriteAsync here so no thread-pool continuation before bytes hit the wire).
                stream.Write(options.ServerList602, 0, options.ServerList602.Length);
                stream.Flush();
                sentListOnAccept = true;
                Console.WriteLine(
                    "[connect] sent {0}: ServerList on-accept stream.Write ({1} bytes)",
                    remote,
                    options.ServerList602.Length);
                Console.Error.WriteLine(
                    "[connect] sent {0}: ServerList on-accept stream.Write ({1} bytes)",
                    remote,
                    options.ServerList602.Length);
                Console.Out.Flush();
                Console.Error.Flush();
                    "[connect] sent {0}: ServerList on-accept ({1} bytes)",
                    remote,
                    options.ServerList602.Length);
            }

            var buf = new byte[512];
            while (!ct.IsCancellationRequested)
            {
                var n = await stream.ReadAsync(buf.AsMemory(0, buf.Length), ct).ConfigureAwait(false);
                if (n <= 0)
                {
                    break;
                }

                var span = buf.AsSpan(0, n);
                var hex = Convert.ToHexString(span);
                Console.WriteLine("[connect] recv {0}: {1}", remote, hex);

                if (ConnectServerPacketClassifier.TryFindFirstRequestOfKind(span, TakumiConnectRequestKind.PatchCheck, out var offPatch, out _))
                {
                    if (offPatch != 0)
                    {
                        Console.WriteLine(
                            "[connect] patch/version probe at offset {0} from {1} (non-zero prefix — check middleboxes / coalesced reads)",
                            offPatch,
                            remote);
                    }

                    var patchOk = ConnectPatchWire602.BuildPatchVersionOkay();
                    await stream.WriteAsync(patchOk, ct).ConfigureAwait(false);
                    await stream.FlushAsync(ct).ConfigureAwait(false);
                    Console.WriteLine("[connect] sent {0}: PatchVersionOkay ({1} bytes)", remote, patchOk.Length);
                }
                else if (ConnectServerPacketClassifier.TryFindFirstRequestOfKind(
                             span,
                             TakumiConnectRequestKind.ServerList,
                             out var offList,
                             out _))
                {
                    if (offList != 0)
                    {
                        Console.WriteLine(
                            "[connect] F4 02|06 at offset {0} from {1} (non-zero prefix — check middleboxes / coalesced reads)",
                            offList,
                            remote);
                    }

                    if (options.ReturnBusy)
                    {
                        var busy = ConnectServerBusy602.Build(options.BusyServerIndex);
                        await stream.WriteAsync(busy, ct).ConfigureAwait(false);
                        await stream.FlushAsync(ct).ConfigureAwait(false);
                        Console.WriteLine(
                            "[connect] sent {0}: ServerBusy F4 05 index={1} ({2} bytes)",
                            remote,
                            options.BusyServerIndex,
                            busy.Length);
                    }
                    else if (!sentListOnAccept)
                    {
                        await stream.WriteAsync(options.ServerList602, ct).ConfigureAwait(false);
                        await stream.FlushAsync(ct).ConfigureAwait(false);
                        Console.WriteLine(
                            "[connect] sent {0}: ServerList ({1} bytes)",
                            remote,
                            options.ServerList602.Length);
                    }
                    else if (options.Verbose)
                    {
                        Console.WriteLine(
                            "[connect] {0}: F4 02|06 after on-accept list — skipping duplicate ServerList",
                            remote);
                    }
                }
                else if (ConnectServerPacketClassifier.TryFindFirstRequestOfKind(
                             span,
                             TakumiConnectRequestKind.ServerInfo,
                             out var offInfo,
                             out _))
                {
                    if (offInfo != 0)
                    {
                        Console.WriteLine("[connect] F4 03 at offset {0} from {1}", offInfo, remote);
                    }

                    var pkt = ConnectServerInfo602.Build(options.PublicHost, options.GamePort);
                    await stream.WriteAsync(pkt, ct).ConfigureAwait(false);
                    await stream.FlushAsync(ct).ConfigureAwait(false);
                    Console.WriteLine(
                        "[connect] sent {0}: ServerInfo ip={1} port={2} ({3} bytes)",
                        remote,
                        options.PublicHost,
                        options.GamePort,
                        pkt.Length);
                }
                else if (options.Verbose)
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
            Console.Error.WriteLine("[connect] error {0}: {1}", remote, ex.Message);
            Console.Out.Flush();
            Console.Error.Flush();
        }
        finally
        {
            tcp.Dispose();
        }
    }
}
