using Takumi.Server.Game;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class GameInGameRegistrationTests
{
    [Fact]
    public void RegisterAccount_adds_new_login_pair()
    {
        var accounts = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
        {
            ["test"] = "test",
        };

        var req = new GameInGameRegistration.RegisterRequest("test1", "pass", "1234567", "0332036526");
        var result = GameInGameRegistration.RegisterAccount(accounts, req);

        Assert.Equal(GameInGameRegistration.ResultSuccess, result);
        Assert.Equal("pass", accounts["test1"]);
    }

    [Fact]
    public void RegisterAccount_rejects_duplicate()
    {
        var accounts = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
        {
            ["test1"] = "old",
        };

        var req = new GameInGameRegistration.RegisterRequest("test1", "new", "1234567", "0332036526");
        var result = GameInGameRegistration.RegisterAccount(accounts, req);

        Assert.Equal(GameInGameRegistration.ResultAccountExists, result);
        Assert.Equal("old", accounts["test1"]);
    }

    [Fact]
    public void BuildResponse_frame_is_C1_D3_05()
    {
        var frame = GameInGameRegistration.BuildResponse(GameInGameRegistration.ResultSuccess);
        Assert.Equal(new byte[] { 0xC1, 0x08, 0xD3, 0x05, 0x01, 0x00, 0x00, 0x00 }, frame);
    }
}
