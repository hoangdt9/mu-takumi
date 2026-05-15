namespace Takumi.Server.Persistence;

/// <summary>
/// One character row for JSON ↔ Postgres roster bridge (M4b).
/// <see cref="PosX"/> / <see cref="PosY"/> are **map tile indices** (0–255), same semantics as walk / join wire — not world floats; see <c>docs/M4-TILE-AND-COORDINATES.md</c>.
/// </summary>
public sealed class CharacterRosterRow
{
    public string Name { get; set; } = "";

    public byte ServerClass { get; set; }

    public ushort Level { get; set; }

    public byte MapId { get; set; }

    public byte PosX { get; set; }

    public byte PosY { get; set; }

    public byte Angle { get; set; }
}
