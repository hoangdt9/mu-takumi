using System.Net.Sockets;
using MUnique.OpenMU.Network.SimpleModulus;

namespace Takumi.Server.Game;

/// <summary>Accept loop for dedicated game TCP (M6).</summary>
public static class GameListenHost
{
    public static async Task RunAsync(int listenPort, GamePortListenOptions options, CancellationToken cancellationToken)
    {
        using var listener = new TcpListener(System.Net.IPAddress.Any, listenPort);
        listener.Server.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, options.ReuseAddress);
        listener.Start();
        Console.WriteLine("[game-host] listening on *:{0} (reuseAddr={1})", listenPort, options.ReuseAddress);

        var useMinimal = options.AuthAccounts is { Count: > 0 }
                         && options.AuthServerSerial16 is { Length: 16 };

        if (useMinimal)
        {
            Console.WriteLine("[game-host] mode=minimal-login (F1 01 + F3 00 + F3 03 join from takumi-roster JSON)");
        }
        else
        {
            Console.WriteLine(
                "[game-host] mode=bootstrap-only (set TAKUMI_ACCOUNTS + TAKUMI_SERVER_SERIAL for full split-port login)");
        }

        while (!cancellationToken.IsCancellationRequested)
        {
            TcpClient tcp;
            try
            {
                tcp = await listener.AcceptTcpClientAsync(cancellationToken).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested)
            {
                break;
            }

            var remote = tcp.Client.RemoteEndPoint?.ToString() ?? "?";
            if (useMinimal)
            {
                _ = GamePortMinimalSession.RunAsync(tcp, remote, options, cancellationToken);
            }
            else
            {
                _ = GamePortBootstrapSession.RunAsync(tcp, remote, options, cancellationToken);
            }
        }
    }
}
