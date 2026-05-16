using Takumi.Server.Hosting;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class DockerRuntimeEnvTests
{
    [Fact]
    public void ApplyStackOverrides_rewrites_localhost_pg_and_clears_game_handoff()
    {
        var keys = new[]
        {
            "TAKUMI_DOCKER_STACK",
            "TAKUMI_DOCKER_GAMEHOST",
            "TAKUMI_DOCKER_GAMEHOST_REQUIRE_HANDOFF",
            "TAKUMI_PG_CONNECTION_STRING",
            "TAKUMI_PG_HOST",
            "TAKUMI_PG_PORT",
            "TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF",
            "TAKUMI_GAME_TICKET_WIRE",
        };
        var saved = keys.ToDictionary(k => k, k => Environment.GetEnvironmentVariable(k));
        try
        {
            Environment.SetEnvironmentVariable("TAKUMI_DOCKER_STACK", "1");
            Environment.SetEnvironmentVariable("TAKUMI_DOCKER_GAMEHOST", "1");
            Environment.SetEnvironmentVariable("TAKUMI_DOCKER_GAMEHOST_REQUIRE_HANDOFF", null);
            Environment.SetEnvironmentVariable("TAKUMI_PG_CONNECTION_STRING", "Host=127.0.0.1;Port=54444;Username=takumi;Password=takumi;Database=takumi_runtime");
            Environment.SetEnvironmentVariable("TAKUMI_PG_USER", "postgres");
            Environment.SetEnvironmentVariable("TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF", "1");
            Environment.SetEnvironmentVariable("TAKUMI_GAME_TICKET_WIRE", "1");

            DockerRuntimeEnv.ApplyStackOverridesIfEnabled();

            Assert.Equal("postgres", Environment.GetEnvironmentVariable("TAKUMI_PG_HOST"));
            Assert.Equal("takumi", Environment.GetEnvironmentVariable("TAKUMI_PG_USER"));
            Assert.Equal("5432", Environment.GetEnvironmentVariable("TAKUMI_PG_PORT"));
            Assert.Null(Environment.GetEnvironmentVariable("TAKUMI_PG_CONNECTION_STRING"));
            Assert.Null(Environment.GetEnvironmentVariable("TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF"));
            Assert.Null(Environment.GetEnvironmentVariable("TAKUMI_GAME_TICKET_WIRE"));
        }
        finally
        {
            foreach (var (k, v) in saved)
            {
                Environment.SetEnvironmentVariable(k, v);
            }
        }
    }
}
