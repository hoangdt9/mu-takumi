using Takumi.Server.Game;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class GamePacketFindersTests
{
    [Fact]
    public void Plain_ping_C1_03_71_is_detected()
    {
        var packet = new byte[] { 0xC1, 0x03, 0x71 };
        Assert.True(GamePacketFinders.TryFindPingResponse(packet, out var off));
        Assert.Equal(0, off);
        Assert.False(GamePacketFinders.TryFindCharacterListRequest(packet, out _));
    }

    [Fact]
    public void Plain_character_list_C1_05_F3_00_is_detected()
    {
        var packet = new byte[] { 0xC1, 0x05, 0xF3, 0x00, 0x00 };
        Assert.True(GamePacketFinders.TryFindCharacterListRequest(packet, out var off));
        Assert.Equal(0, off);
        Assert.False(GamePacketFinders.TryFindPingResponse(packet, out _));
    }

    [Fact]
    public void C3_ping_with_serial_byte_is_not_character_list()
    {
        var packet = new byte[] { 0xC3, 0x0C, 0x01, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        Assert.True(GamePacketFinders.TryFindPingResponse(packet, out _));
        Assert.False(GamePacketFinders.TryFindCharacterListRequest(packet, out _));
    }

    [Fact]
    public void C3_character_list_with_serial_byte_is_detected()
    {
        var packet = new byte[] { 0xC3, 0x0C, 0x01, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        Assert.True(GamePacketFinders.TryFindCharacterListRequest(packet, out _));
        Assert.False(GamePacketFinders.TryFindPingResponse(packet, out _));
    }
}
