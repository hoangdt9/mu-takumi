using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class PostgresConnectionStringEnvTests
{
    [Theory]
    [InlineData("postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime", true)]
    [InlineData("Host=127.0.0.1;Port=54444;Username=takumi;Password=takumi;Database=takumi_runtime", true)]
    [InlineData("Host=127.0.0.1", false)]
    [InlineData("Host=127.0.0.1;Port=54444", false)]
    public void LooksLikeCompletePostgresConnection(string value, bool expected) =>
        Assert.Equal(expected, PostgresCharacterRosterRepository.LooksLikeCompletePostgresConnection(value));
}
