namespace Takumi.Server.Protocol;

/// <summary>
/// Takumi <c>PRECEIVE_SERVER_BUSY</c> (<c>WSclient.h</c>): <c>C1 05 F4 05</c> + server index; client calls <c>SendRequestServerList()</c>.
/// </summary>
public static class ConnectServerBusy602
{
    public static byte[] Build(byte serverIndex) => new byte[] { 0xC1, 0x05, 0xF4, 0x05, serverIndex };
}
