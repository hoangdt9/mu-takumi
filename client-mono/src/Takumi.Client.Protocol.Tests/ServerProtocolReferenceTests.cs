using Takumi.Client.Protocol;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Client.Protocol.Tests;

public sealed class ServerProtocolReferenceTests
{
    [Fact]
    public void Client_references_server_character_list_golden()
    {
        Assert.Equal(
            new byte[] { 0xC1, 0x08, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00 },
            CharacterListWire602.BuildEmpty());
    }

    [Fact]
    public void TryReadC1_parses_login_result()
    {
        var packet = LoginAccountWire602.BuildLoginResult(0x01).ToArray();
        Assert.True(ClientPacketEnvelope.TryReadC1(packet, out var len, out var head, out var sub));
        Assert.Equal(5, len);
        Assert.Equal(Season6HeadCodes.JoinServer, head);
        Assert.Equal(0x01, sub);
    }
}
