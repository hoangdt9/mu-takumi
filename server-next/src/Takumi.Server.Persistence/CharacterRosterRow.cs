namespace Takumi.Server.Persistence;

/// <summary>One character row for JSON ↔ Postgres roster bridge (M4b).</summary>
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
