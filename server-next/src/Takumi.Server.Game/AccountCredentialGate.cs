using Takumi.Server.Persistence;

namespace Takumi.Server.Game;

/// <summary>Login + in-game register: Postgres <c>account</c> when enabled, else in-memory <c>TAKUMI_ACCOUNTS</c>.</summary>
public static class AccountCredentialGate
{
    public static bool IsDbEnabled => TakumiPostgresMirror.Accounts is not null;

    public static async Task<bool> TryValidateLoginAsync(
        string accountLogin,
        string password,
        IReadOnlyDictionary<string, string> envFallback,
        CancellationToken ct = default)
    {
        if (TakumiPostgresMirror.Accounts is { } repo)
        {
            try
            {
                if (await repo.VerifyPasswordAsync(accountLogin, password, ct).ConfigureAwait(false))
                {
                    return true;
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("[account-db] verify failed for '{0}': {1}", accountLogin, ex.Message);
                return false;
            }
        }

        return envFallback.TryGetValue(accountLogin, out var envPass)
               && string.Equals(envPass, password, StringComparison.Ordinal);
    }

    public static async Task<byte> RegisterAsync(
        GameInGameRegistration.RegisterRequest request,
        IDictionary<string, string> envMemory,
        CancellationToken ct = default)
    {
        if (!GameInGameRegistration.IsValidRequest(request))
        {
            return GameInGameRegistration.ResultInvalidInput;
        }

        if (TakumiPostgresMirror.Accounts is { } repo)
        {
            try
            {
                if (await repo.ExistsAsync(request.Account, ct).ConfigureAwait(false))
                {
                    return GameInGameRegistration.ResultAccountExists;
                }

                var inserted = await repo.TryCreateAsync(
                        request.Account,
                        request.Password,
                        request.SecurityCode,
                        request.Phone,
                        ct)
                    .ConfigureAwait(false);
                if (!inserted)
                {
                    return GameInGameRegistration.ResultAccountExists;
                }

                envMemory[request.Account] = request.Password;
                return GameInGameRegistration.ResultSuccess;
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("[account-db] register failed for '{0}': {1}", request.Account, ex.Message);
                return GameInGameRegistration.ResultInvalidInput;
            }
        }

        return GameInGameRegistration.RegisterAccount(envMemory, request);
    }

    public static async Task SeedEnvAccountsAsync(
        IReadOnlyDictionary<string, string> envAccounts,
        CancellationToken ct = default)
    {
        if (TakumiPostgresMirror.Accounts is not { } repo || envAccounts.Count == 0)
        {
            return;
        }

        foreach (var pair in envAccounts)
        {
            try
            {
                await repo.SeedIfMissingAsync(pair.Key, pair.Value, ct).ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("[account-db] seed skip '{0}': {1}", pair.Key, ex.Message);
            }
        }
    }
}
