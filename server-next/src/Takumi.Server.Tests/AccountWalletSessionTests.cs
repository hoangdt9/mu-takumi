using Takumi.Server.Game;
using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class AccountWalletSessionTests
{
    [Fact]
    public void Warehouse_transfer_deposit_and_withdraw()
    {
        var player = new GameRosterEntry { Zen = 1_000_000 };
        const string account = "test_wallet";

        Assert.True(AccountWalletSession.TryTransferWarehouseZen(account, player, flag: 0, 400_000, out var inv, out var wh));
        Assert.Equal(600_000u, inv);
        Assert.Equal(400_000u, wh);
        Assert.Equal(600_000, player.Zen);

        Assert.True(AccountWalletSession.TryTransferWarehouseZen(account, player, flag: 1, 100_000, out inv, out wh));
        Assert.Equal(700_000u, inv);
        Assert.Equal(300_000u, wh);
        Assert.Equal(700_000, player.Zen);
    }

    [Fact]
    public void Warehouse_withdraw_respects_two_billion_carry_cap()
    {
        var player = new GameRosterEntry { Zen = 2_000_000_000 };
        const string account = "test_wallet_2b";

        Assert.True(AccountWalletSession.TryTransferWarehouseZen(account, player, flag: 0, 2_000_000_000, out _, out var wh));
        Assert.Equal(0, player.Zen);
        Assert.Equal(2_000_000_000u, wh);

        Assert.True(AccountWalletSession.TryTransferWarehouseZen(account, player, flag: 1, 2_000_000_000, out var inv, out wh));
        Assert.Equal(2_000_000_000, player.Zen);
        Assert.Equal(2_000_000_000u, inv);
        Assert.Equal(0u, wh);

        Assert.False(AccountWalletSession.TryTransferWarehouseZen(account, player, flag: 1, 1, out _, out _));
    }

    [Fact]
    public void Coin_debit_fails_when_balance_insufficient()
    {
        const string account = "test_coin";
        Assert.False(AccountWalletSession.TryDebitCoin(account, 1, 50, out var reason));
        Assert.Equal("insufficient-coin", reason);
        Assert.True(AccountWalletSession.TryGetCoinBalance(account, 1, out var balance));
        Assert.Equal(0, balance);
    }
}
