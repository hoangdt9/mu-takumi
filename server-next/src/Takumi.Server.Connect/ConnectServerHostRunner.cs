using System.Globalization;
using System.Net;
using System.Net.Sockets;
using Takumi.Server.Hosting;
using Takumi.Server.Protocol;

namespace Takumi.Server.Connect;

/// <summary>M5: standalone Connect Server process (<c>1.ConnectServer</c> parity) — F4 06/03 only.</summary>
public static class ConnectServerHostRunner
{
    public static async Task<int> RunAsync(CancellationToken appCancellationToken)
    {
        RepoEnvLoader.ApplyDefaultsAndLocalEnv();

        if (!int.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_CONNECT_PORT"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var connectPort)
            || connectPort is <= 0 or > 65535)
        {
            Console.Error.WriteLine(
                "Missing or invalid TAKUMI_CONNECT_PORT. Set it in server-next/env.defaults or .env.");
            return 1;
        }

        var loginPort = connectPort;
        if (int.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_LOGIN_PORT"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var lp)
            && lp is > 0 and <= 65535)
        {
            loginPort = lp;
        }

        var advertisedGamePort = loginPort;
        var gamePortRaw = Environment.GetEnvironmentVariable("TAKUMI_GAME_PORT")?.Trim();
        if (!string.IsNullOrEmpty(gamePortRaw)
            && int.TryParse(gamePortRaw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var gpParsed)
            && gpParsed is > 0 and <= 65535)
        {
            advertisedGamePort = gpParsed;
        }

        var verbose = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_VERBOSE"), "1", StringComparison.OrdinalIgnoreCase)
                      || string.Equals(Environment.GetEnvironmentVariable("DOTNET_ENVIRONMENT"), "Development", StringComparison.OrdinalIgnoreCase);

        var publicHost = Environment.GetEnvironmentVariable("TAKUMI_PUBLIC_HOST")?.Trim();
        if (string.IsNullOrEmpty(publicHost))
        {
            publicHost = Environment.GetEnvironmentVariable("TAKUMI_LAN_IP")?.Trim();
        }

        if (string.IsNullOrEmpty(publicHost))
        {
            Console.Error.WriteLine(
                "[connect] WARNING: TAKUMI_LAN_IP unset — using 127.0.0.1. Set server-next/.env for LAN phones.");
            publicHost = "127.0.0.1";
        }

        var (connectServerListPacket, connectListBootDesc) = BuildConnectServerListPacket();
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

        var reuseSocketAddr = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_REUSE_ADDR"), "1", StringComparison.OrdinalIgnoreCase);

        using var cts = CancellationTokenSource.CreateLinkedTokenSource(appCancellationToken);
        Console.CancelKeyPress += (_, e) => { e.Cancel = true; cts.Cancel(); };

        TcpListener connectListener;
        try
        {
            connectListener = new TcpListener(IPAddress.Any, connectPort);
            connectListener.Server.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, reuseSocketAddr);
            connectListener.Start();
        }
        catch (SocketException ex) when (ex.SocketErrorCode == SocketError.AddressAlreadyInUse)
        {
            Console.Error.WriteLine(
                "Cannot bind connect port {0}: address already in use.\n  Check: lsof -nP -iTCP:{0} -sTCP:LISTEN",
                connectPort);
            return 1;
        }

        var connectOptions = new ConnectMiniServerOptions
        {
            PublicHost = publicHost,
            GamePort = (ushort)advertisedGamePort,
            Verbose = verbose,
            ServerList602 = connectServerListPacket,
            SendServerListOnAccept = connectSendListOnAccept,
            ReturnBusy = connectReturnBusy,
            BusyServerIndex = connectBusyServerIndex,
        };

        Console.WriteLine(
            "Takumi.Server.ConnectHost listening on *:{0} (M5 split). F4 03 → {1}:{2} (login listen={3}). Ctrl+C to stop.\n" +
            "  F4 06 payload: {4}",
            connectPort,
            publicHost,
            advertisedGamePort,
            loginPort,
            connectListBootDesc);

        try
        {
            await ConnectMiniServer.RunAcceptLoopAsync(connectListener, connectOptions, cts.Token).ConfigureAwait(false);
        }
        catch (OperationCanceledException) when (cts.IsCancellationRequested)
        {
        }
        finally
        {
            try
            {
                connectListener.Stop();
            }
            catch
            {
                // ignore shutdown races
            }
        }

        return 0;
    }

    static (byte[] Packet, string Description) BuildConnectServerListPacket()
    {
        var csBaseRaw = Environment.GetEnvironmentVariable("TAKUMI_CS_CONNECT_BASE");
        var csCountRaw = Environment.GetEnvironmentVariable("TAKUMI_CS_CONNECT_COUNT");
        var csIdsRaw = Environment.GetEnvironmentVariable("TAKUMI_CS_CONNECT_IDS");

        if (TryParseConnectIdsCsv(csIdsRaw, out var explicitConnectIds))
        {
            return (
                ConnectServerList602.BuildFromIds(explicitConnectIds, loadPercent: 0x0A),
                $"TAKUMI_CS_CONNECT_IDS=[{string.Join(',', explicitConnectIds)}]");
        }

        if (!string.IsNullOrWhiteSpace(csBaseRaw) || !string.IsNullOrWhiteSpace(csCountRaw))
        {
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
                        "[connect] WARNING: TAKUMI_CS_CONNECT_BASE=0 invalid for typical ServerList.bmd — using 20.");
                }
            }

            var csConnectCount = 3;
            if (!string.IsNullOrWhiteSpace(csCountRaw)
                && int.TryParse(csCountRaw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var csc)
                && csc is >= 1 and <= 32)
            {
                csConnectCount = csc;
            }

            return (
                ConnectServerList602.Build(csConnectBase, csConnectCount, loadPercent: 0x0A),
                $"TAKUMI_CS_CONNECT_BASE={csConnectBase} TAKUMI_CS_CONNECT_COUNT={csConnectCount}");
        }

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
        return (
            ConnectServerList602.BuildFromIds(preset, loadPercent: 0x0A),
            "default F4 06: 15+15+2 safe ids (0..14,20..34,40,41)");
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
            Console.Error.WriteLine("[connect] WARNING: TAKUMI_CS_CONNECT_IDS must have 1..32 integers.");
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
}
