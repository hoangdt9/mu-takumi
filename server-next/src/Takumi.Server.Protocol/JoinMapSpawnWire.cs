namespace Takumi.Server.Protocol;

/// <summary>
/// Spawn slice of <c>PRECEIVE_JOIN_MAP_SERVER</c> (map id + **tile** XY + facing). Coordinates are tile indices (0–255), not world-space floats; see <c>docs/M4-TILE-AND-COORDINATES.md</c>.
/// </summary>
public readonly record struct JoinMapSpawnWire(byte Map, byte PositionX, byte PositionY, byte Angle)
{
    /// <summary>Lorencia town gate-ish defaults used by the legacy minimal host (see Takumi <c>ReceiveJoinMapServer</c>).</summary>
    public static JoinMapSpawnWire LorenciaDefault => new(0, 135, 122, 1);
}
