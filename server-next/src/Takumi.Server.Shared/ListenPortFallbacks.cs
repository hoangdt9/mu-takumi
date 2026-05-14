namespace Takumi.Server.Shared;

/// <summary>Defaults when <c>TAKUMI_*_PORT</c> env vars are unset. Override via Docker, <c>server-next/.env</c>, or shell.</summary>
public static class ListenPortFallbacks
{
    public const int ConnectServer = 44605;

    public const int LegacyLogin = 44606;

    public const int GameTcp = 55901;
}
