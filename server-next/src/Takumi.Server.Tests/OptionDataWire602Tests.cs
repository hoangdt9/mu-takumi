using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class OptionDataWire602Tests
{
    [Fact]
    public void BuildApply_has_expected_header_and_length()
    {
        var config = CharacterKeyConfiguration.CreateDefault();
        config[0] = 0x12;
        config[1] = 0x34;

        var pkt = OptionDataWire602.BuildApply(config);

        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(34, pkt[1]);
        Assert.Equal(0xF3, pkt[2]);
        Assert.Equal(0x30, pkt[3]);
        Assert.Equal(0x12, pkt[4]);
        Assert.Equal(0x34, pkt[5]);
    }

    [Fact]
    public void TryFindSaveRequest_round_trips_client_frame()
    {
        var config = CharacterKeyConfiguration.CreateDefault();
        var frame = OptionDataWire602.BuildApply(config);

        Assert.True(OptionDataWire602.TryFindSaveRequest(frame, out var parsed));
        Assert.Equal(config.Length, parsed.Length);
        Assert.True(parsed.SequenceEqual(config));
    }
}
