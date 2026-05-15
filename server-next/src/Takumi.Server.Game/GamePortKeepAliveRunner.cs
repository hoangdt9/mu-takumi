using System.Globalization;
using System.Net.Sockets;
using MUnique.OpenMU.Network;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>Periodic <c>C1 03 71</c> ping on the game/login TCP (same as <c>LegacyLoginHost</c>).</summary>
public static class GamePortKeepAliveRunner
{
    public static TimeSpan ParseIntervalSeconds()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_GAME_KEEPALIVE_SECONDS");
        if (string.IsNullOrWhiteSpace(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var sec))
        {
            return TimeSpan.FromSeconds(25);
        }

        if (sec <= 0)
        {
            return TimeSpan.Zero;
        }

        sec = Math.Clamp(sec, 5, 600);
        return TimeSpan.FromSeconds(sec);
    }

    public static async Task RunAsync(
        Connection connection,
        SemaphoreSlim packetGate,
        Func<bool> isLoggedIn,
        TcpClient tcp,
        string remote,
        bool verbose,
        TimeSpan interval,
        (byte EncDecKey1, byte EncDecKey2)? clientProtectOutbound,
        CancellationToken cancellationToken)
    {
        try
        {
            var pollWhenNotLoggedIn = TimeSpan.FromSeconds(3);
            while (!cancellationToken.IsCancellationRequested)
            {
                if (!isLoggedIn() || !tcp.Connected)
                {
                    await Task.Delay(pollWhenNotLoggedIn, cancellationToken).ConfigureAwait(false);
                    continue;
                }

                await packetGate.WaitAsync(cancellationToken).ConfigureAwait(false);
                try
                {
                    await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, GamePortKeepAliveWire.PingRequest, cancellationToken).ConfigureAwait(false);
                    if (verbose)
                    {
                        Console.WriteLine("[{0}] keepalive sent C1 03 71 (ping request)", remote);
                    }
                }
                catch (ObjectDisposedException)
                {
                    break;
                }
                finally
                {
                    packetGate.Release();
                }

                await Task.Delay(interval, cancellationToken).ConfigureAwait(false);
            }
        }
        catch (OperationCanceledException)
        {
        }
        catch (Exception ex)
        {
            Console.WriteLine("[{0}] keepalive task error: {1}", remote, ex.Message);
        }
    }
}
