namespace Takumi.Server.Persistence;

/// <summary>One row in <c>public.character_skill</c> (join <c>F3 11</c> list entry).</summary>
public sealed class CharacterSkillRow
{
    public byte Slot { get; init; }

    public ushort Type { get; init; }

    public byte Level { get; init; }
}
