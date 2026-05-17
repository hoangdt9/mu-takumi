using System.Collections.Concurrent;
using Takumi.Server.Persistence;

namespace Takumi.Server.Game.World;

/// <summary>Per-account warehouse zen + coin balances (memory + optional Postgres mirror).</summary>
public static class AccountWalletSession
{
    const long MaxCarryZen = 2_000_000_000;

    static readonly ConcurrentDictionary<string, AccountWalletRow> Cache = new(StringComparer.OrdinalIgnoreCase);

    public static async Task EnsureLoadedAsync(string? accountLogin, CancellationToken ct)
    {
        if (string.IsNullOrWhiteSpace(accountLogin))
        {
            return;
        }

        var key = NormaliseAccount(accountLogin);
        if (Cache.ContainsKey(key))
        {
            return;
        }

        var repo = TakumiPostgresMirror.AccountWallet;
        if (repo is null)
        {
            Cache.TryAdd(key, new AccountWalletRow());
            return;
        }

        try
        {
            var row = await repo.TryLoadAsync(key, ct).ConfigureAwait(false);
            Cache[key] = row ?? new AccountWalletRow();
        }
        catch (Exception ex)
        {
            Console.WriteLine("[wallet] load failed account={0}: {1}", key, ex.Message);
            Cache.TryAdd(key, new AccountWalletRow());
        }
    }

    public static long GetWarehouseZen(string? accountLogin)
    {
        if (string.IsNullOrWhiteSpace(accountLogin))
        {
            return 0;
        }

        var key = NormaliseAccount(accountLogin);
        return Cache.TryGetValue(key, out var row) ? row.WarehouseZen : 0;
    }

    public static bool TryGetCoinBalance(string? accountLogin, int priceType, out long balance)
    {
        balance = 0;
        if (string.IsNullOrWhiteSpace(accountLogin))
        {
            return false;
        }

        var key = NormaliseAccount(accountLogin);
        if (!Cache.TryGetValue(key, out var row))
        {
            return false;
        }

        balance = priceType switch
        {
            1 => row.WCoinC,
            2 => row.WCoinP,
            3 => row.GoblinPoint,
            _ => 0,
        };
        return priceType is 1 or 2 or 3;
    }

    public static bool TryDebitCoin(string? accountLogin, int priceType, long amount, out string reason)
    {
        reason = string.Empty;
        if (amount <= 0 || priceType is < 1 or > 3)
        {
            reason = "invalid-coin-price";
            return false;
        }

        if (string.IsNullOrWhiteSpace(accountLogin))
        {
            reason = "no-account";
            return false;
        }

        var key = NormaliseAccount(accountLogin);
        var row = Cache.GetOrAdd(key, _ => new AccountWalletRow());
        var balance = priceType switch
        {
            1 => row.WCoinC,
            2 => row.WCoinP,
            3 => row.GoblinPoint,
            _ => 0,
        };
        if (balance < amount)
        {
            reason = "insufficient-coin";
            return false;
        }

        var updated = priceType switch
        {
            1 => row with { WCoinC = balance - amount },
            2 => row with { WCoinP = balance - amount },
            3 => row with { GoblinPoint = balance - amount },
            _ => row,
        };
        Cache[key] = updated;
        SchedulePersist(key, updated);
        return true;
    }

    public static bool TryTransferWarehouseZen(
        string? accountLogin,
        GameRosterEntry player,
        byte flag,
        uint amount,
        out uint inventoryZen,
        out uint warehouseZen)
    {
        inventoryZen = 0;
        warehouseZen = 0;
        if (amount == 0 || string.IsNullOrWhiteSpace(accountLogin))
        {
            return false;
        }

        var key = NormaliseAccount(accountLogin);
        var row = Cache.GetOrAdd(key, _ => new AccountWalletRow());
        var inv = player.Zen;
        var wh = row.WarehouseZen;

        if (flag == 0)
        {
            if (inv < amount)
            {
                return false;
            }

            inv -= amount;
            wh += amount;
        }
        else if (flag == 1)
        {
            if (wh < amount || inv + amount > MaxCarryZen)
            {
                return false;
            }

            wh -= amount;
            inv += amount;
        }
        else
        {
            return false;
        }

        player.Zen = inv;
        var updated = row with { WarehouseZen = wh };
        Cache[key] = updated;
        SchedulePersist(key, updated);
        inventoryZen = (uint)Math.Clamp(inv, 0, uint.MaxValue);
        warehouseZen = (uint)Math.Clamp(wh, 0, uint.MaxValue);
        return true;
    }

    static string NormaliseAccount(string accountLogin) =>
        accountLogin.Trim().ToLowerInvariant();

    static void SchedulePersist(string accountLogin, AccountWalletRow row)
    {
        var repo = TakumiPostgresMirror.AccountWallet;
        if (repo is null)
        {
            return;
        }

        _ = Task.Run(async () =>
        {
            try
            {
                await repo.SaveAsync(accountLogin, row, CancellationToken.None).ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                Console.WriteLine("[wallet] persist failed account={0}: {1}", accountLogin, ex.Message);
            }
        });
    }
}
