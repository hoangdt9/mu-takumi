using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>Character row for Season 6 list / join wire builders (Takumi <c>PRECEIVE_CHARACTER_LIST</c> / join stub).</summary>
public sealed class CharacterRosterWire
{
    public CharacterRosterWire(
        ReadOnlySpan<byte> name10,
        byte serverClass,
        ushort level,
        CharacterRosterVitals vitals = default,
        CharacterSheetStats sheet = default)
    {
        this.Name10 = new byte[10];
        var n = Math.Min(10, name10.Length);
        name10[..n].CopyTo(this.Name10);
        this.ServerClass = serverClass;
        this.Level = level;
        this.Vitals = vitals;
        this.Sheet = sheet;
    }

    public byte[] Name10 { get; }

    public byte ServerClass { get; }

    public ushort Level { get; }

    /// <summary>M7: when <see cref="CharacterRosterVitals.HasHp"/> / <see cref="CharacterRosterVitals.HasMp"/>, join stats use these instead of the level stub.</summary>
    public CharacterRosterVitals Vitals { get; }

    /// <summary>M7: base stats + level-up points; unset → class defaults from <see cref="CharacterSheetCalculator"/>.</summary>
    public CharacterSheetStats Sheet { get; }
}
