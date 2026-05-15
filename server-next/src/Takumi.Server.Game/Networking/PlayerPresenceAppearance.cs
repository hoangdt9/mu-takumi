namespace Takumi.Server.Game.Networking;

/// <summary>Roster fields needed to build <c>C2 0x12</c> for other players.</summary>
public sealed class PlayerPresenceAppearance
{
    public byte[] Name10 { get; init; } = new byte[10];

    public byte ServerClass { get; init; }

    public byte PkLevel { get; init; }
}
