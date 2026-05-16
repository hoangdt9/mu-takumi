using Takumi.Server.Protocol;

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

    /// <summary>M7: cumulative EXP (0 = new character).</summary>
    public long Experience { get; set; }

    public byte MapId { get; set; }

    public byte PosX { get; set; }

    public byte PosY { get; set; }

    public byte Angle { get; set; }

    /// <summary>M7: 0 = unset (join uses stub). See <c>sql/init/004_character_roster_vitals.sql</c>.</summary>
    public int CurrentHp { get; set; }

    public int MaxHp { get; set; }

    public int CurrentMp { get; set; }

    public int MaxMp { get; set; }

    public long Zen { get; set; }

    /// <summary>M7: SD current (0 = treat as full when <see cref="MaxShield"/> &gt; 0 at join).</summary>
    public int CurrentShield { get; set; }

    public int MaxShield { get; set; }

    public int Strength { get; set; }

    public int Dexterity { get; set; }

    public int Vitality { get; set; }

    public int Energy { get; set; }

    public int Leadership { get; set; }

    public int LevelUpPoint { get; set; }

    public int CurrentBp { get; set; }

    public int MaxBp { get; set; }

    public CharacterSheetStats ToSheet() =>
        CharacterSheetStats.FromInts(
            this.Strength,
            this.Dexterity,
            this.Vitality,
            this.Energy,
            this.Leadership,
            this.LevelUpPoint);

    public CharacterRosterVitals ToVitals() =>
        CharacterRosterVitals.FromInts(
            this.CurrentHp,
            this.MaxHp,
            this.CurrentMp,
            this.MaxMp,
            this.Zen,
            this.CurrentShield,
            this.MaxShield);
}

