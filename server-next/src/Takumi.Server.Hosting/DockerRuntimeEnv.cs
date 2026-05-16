namespace Takumi.Server.Hosting;

/// <summary>
/// Overrides host-centric <c>.env</c> values after <see cref="RepoEnvLoader"/> when running inside
/// <c>docker-compose</c> (bind-mounted <c>server-next/.env</c> often points Postgres at 127.0.0.1:54444).
/// </summary>
public static class DockerRuntimeEnv
{
    public static void ApplyStackOverridesIfEnabled()
    {
        if (!IsTruthy(Environment.GetEnvironmentVariable("TAKUMI_DOCKER_STACK")))
        {
            return;
        }

        ApplyPostgresServiceEndpoint();

        if (IsTruthy(Environment.GetEnvironmentVariable("TAKUMI_DOCKER_GAMEHOST"))
            && !IsTruthy(Environment.GetEnvironmentVariable("TAKUMI_DOCKER_GAMEHOST_REQUIRE_HANDOFF")))
        {
            // Android M6: F4 03 → game TCP; login happens on game port, not legacy 44606.
            Environment.SetEnvironmentVariable("TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF", null);
            Environment.SetEnvironmentVariable("TAKUMI_GAME_TICKET_WIRE", null);
        }
    }

    static void ApplyPostgresServiceEndpoint()
    {
        var user = Environment.GetEnvironmentVariable("TAKUMI_PG_USER")?.Trim();
        // Host .env often sets postgres (superuser name); compose DB user is takumi.
        if (string.IsNullOrEmpty(user)
            || string.Equals(user, "postgres", StringComparison.OrdinalIgnoreCase))
        {
            user = "takumi";
        }

        var password = Environment.GetEnvironmentVariable("TAKUMI_PG_PASSWORD")?.Trim();
        if (string.IsNullOrEmpty(password))
        {
            password = "takumi";
        }

        var database = Environment.GetEnvironmentVariable("TAKUMI_PG_DATABASE")?.Trim();
        if (string.IsNullOrEmpty(database))
        {
            database = "takumi_runtime";
        }

        Environment.SetEnvironmentVariable("TAKUMI_PG_HOST", "postgres");
        Environment.SetEnvironmentVariable("TAKUMI_PG_PORT", "5432");
        Environment.SetEnvironmentVariable("TAKUMI_PG_USER", user);
        Environment.SetEnvironmentVariable("TAKUMI_PG_PASSWORD", password);
        Environment.SetEnvironmentVariable("TAKUMI_PG_DATABASE", database);
        Environment.SetEnvironmentVariable("TAKUMI_PG_CONNECTION_STRING", null);
    }

    static bool IsTruthy(string? value) =>
        string.Equals(value?.Trim(), "1", StringComparison.OrdinalIgnoreCase)
        || string.Equals(value?.Trim(), "true", StringComparison.OrdinalIgnoreCase);
}
