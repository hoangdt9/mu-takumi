namespace Takumi.Client.Protocol;

/// <summary>
/// Entry point for MonoGame client wire types shared with <c>server-next</c>.
/// Phase 0–1: use server builders for TX; add client-only parsers here as needed.
/// </summary>
public static class ClientWireCatalog
{
    /// <summary>Gate S0 — protocol tests + shared package reference only.</summary>
    public const string CurrentGate = "S0";

    public static Type ConnectListBuilder => typeof(Takumi.Server.Protocol.ConnectServerList602);

    public static Type CharacterListBuilder => typeof(Takumi.Server.Protocol.CharacterListWire602);

    public static Type JoinMapBuilder => typeof(Takumi.Server.Protocol.JoinMapServerWire602);

    public static Type LoginResultBuilder => typeof(Takumi.Server.Protocol.LoginAccountWire602);
}
