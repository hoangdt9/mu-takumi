using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ShopBuyConfirmWire602Tests
{
    [Fact]
    public void Build_confirm_prompt_C1_F3_ED()
    {
        var pkt = ShopBuyConfirmWire602.Build(7);
        Assert.Equal(new byte[] { 0xC1, 0x05, 0xF3, 0xED, 0x07 }, pkt);
    }

    [Fact]
    public void TryFindBuyConfirmRequest_matches_F3_ED()
    {
        var packet = new byte[] { 0xC1, 0x05, 0xF3, 0xED, 0x03 };
        Assert.True(ClientGameplayPackets602.TryFindBuyConfirmRequest(packet, out _, out var slot));
        Assert.Equal(3, slot);
    }
}
