namespace Takumi.Server.Protocol;

/// <summary>
/// Connect-server patch handshake (OpenMU <c>PatchCheckRequest</c> / <c>PatchVersionOkay</c> — head code <c>0x02</c> on <c>C1</c>).
/// </summary>
public static class ConnectPatchWire602
{
    /// <summary>Server → client: patch OK (<c>C1 04 02 00</c> per OpenMU <c>PatchVersionOkay</c> default init).</summary>
    public static byte[] BuildPatchVersionOkay() => new byte[] { 0xC1, 0x04, 0x02, 0x00 };

    /// <summary>Client → server patch/version probe (<c>C1</c> length ≥ 6, head <c>0x02</c>).</summary>
    public static bool IsPatchCheckRequest(ReadOnlySpan<byte> packet)
    {
        if (packet.Length < 6 || packet[0] != 0xC1)
        {
            return false;
        }

        var len = packet[1];
        if (len < 6 || len > packet.Length)
        {
            return false;
        }

        return packet[2] == 0x02;
    }
}
