using Takumi.Server.Protocol;

namespace Takumi.Server.Connect;

/// <summary>Classified minimal connect-server request (aligned with <c>Source/1.ConnectServer/ConnectServerProtocol.cpp</c> + OpenMU Takumi extensions for <c>F4 02</c> / patch).</summary>
public enum TakumiConnectRequestKind
{
    Unknown = 0,
    /// <summary><c>C1</c> patch / version probe (head <c>0x02</c>).</summary>
    PatchCheck,
    /// <summary><c>F4 02</c> or <c>F4 06</c> server list request.</summary>
    ServerList,
    /// <summary><c>F4 03</c> server info request.</summary>
    ServerInfo,
}

/// <summary>
/// Ported rules from <c>MUnique.OpenMU.Takumi.Networking.ConnectServer.TakumiConnectServerProtocol</c> and legacy <c>ConnectServerProtocol.cpp</c> (<c>0xF4</c> only <c>03</c>/<c>06</c> there; <c>02</c> added for clients that use old list opcode).
/// </summary>
public static class ConnectServerPacketClassifier
{
    private const byte HeaderC1 = 0xC1;
    private const byte HeaderC2 = 0xC2;
    private const byte PatchMainCode = 0x05;
    private const byte ServerMainCode = 0xF4;

    /// <summary>Classify a single frame starting at offset 0 of <paramref name="packet"/>.</summary>
    public static TakumiConnectRequestKind Classify(ReadOnlySpan<byte> packet)
    {
        if (packet.Length < 4)
        {
            return TakumiConnectRequestKind.Unknown;
        }

        var header = packet[0];
        if (header is not (HeaderC1 or HeaderC2))
        {
            return TakumiConnectRequestKind.Unknown;
        }

        if (header == HeaderC1 && ConnectPatchWire602.IsPatchCheckRequest(packet))
        {
            return TakumiConnectRequestKind.PatchCheck;
        }

        var mainCode = packet[2];
        if (mainCode == PatchMainCode)
        {
            return TakumiConnectRequestKind.PatchCheck;
        }

        if (mainCode != ServerMainCode || packet.Length < 4)
        {
            return TakumiConnectRequestKind.Unknown;
        }

        var sub = packet[3];
        return sub switch
        {
            0x02 or 0x06 => TakumiConnectRequestKind.ServerList,
            0x03 => TakumiConnectRequestKind.ServerInfo,
            _ => TakumiConnectRequestKind.Unknown,
        };
    }

    /// <summary>Scan for the first decodable <c>C1</c> frame in a possibly prefixed buffer (junk prefix / coalesced reads).</summary>
    public static bool TryFindC1Frame(ReadOnlySpan<byte> data, out int frameOffset, out ReadOnlySpan<byte> frame)
    {
        frameOffset = 0;
        frame = ReadOnlySpan<byte>.Empty;
        for (var i = 0; i <= data.Length - 4; i++)
        {
            if (data[i] != HeaderC1)
            {
                continue;
            }

            var declaredLen = data[i + 1];
            if (declaredLen < 4 || i + declaredLen > data.Length)
            {
                continue;
            }

            frameOffset = i;
            frame = data.Slice(i, declaredLen);
            return true;
        }

        return false;
    }

    /// <summary>Find first <c>C1</c> frame whose classification is not <see cref="TakumiConnectRequestKind.Unknown"/>.</summary>
    public static bool TryFindFirstRequest(ReadOnlySpan<byte> data, out int frameOffset, out TakumiConnectRequestKind kind)
    {
        kind = TakumiConnectRequestKind.Unknown;
        frameOffset = 0;
        for (var i = 0; i <= data.Length - 4; i++)
        {
            if (data[i] != HeaderC1)
            {
                continue;
            }

            var declaredLen = data[i + 1];
            if (declaredLen < 4 || i + declaredLen > data.Length)
            {
                continue;
            }

            var slice = data.Slice(i, declaredLen);
            var c = Classify(slice);
            if (c == TakumiConnectRequestKind.Unknown)
            {
                continue;
            }

            frameOffset = i;
            kind = c;
            return true;
        }

        return false;
    }

    /// <summary>
    /// Like <see cref="TryFindFirstRequest"/> but only matches <paramref name="kind"/> (supports coalesced buffers where another request appears first).
    /// </summary>
    public static bool TryFindFirstRequestOfKind(
        ReadOnlySpan<byte> data,
        TakumiConnectRequestKind kind,
        out int frameOffset,
        out ReadOnlySpan<byte> frame)
    {
        frameOffset = 0;
        frame = ReadOnlySpan<byte>.Empty;
        for (var i = 0; i <= data.Length - 4; i++)
        {
            if (data[i] != HeaderC1)
            {
                continue;
            }

            var declaredLen = data[i + 1];
            if (declaredLen < 4 || i + declaredLen > data.Length)
            {
                continue;
            }

            var slice = data.Slice(i, declaredLen);
            if (Classify(slice) != kind)
            {
                continue;
            }

            frameOffset = i;
            frame = slice;
            return true;
        }

        return false;
    }
}
