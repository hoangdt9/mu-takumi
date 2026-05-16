namespace Takumi.Server.Protocol;

/// <summary>Base character stats (parity <c>DEFAULT_CLASS_INFO</c> + <c>lpObj->Strength</c>…).</summary>
public readonly record struct CharacterSheetStats
{
    public static CharacterSheetStats Unset => default;

    public ushort Strength { get; init; }
    public ushort Dexterity { get; init; }
    public ushort Vitality { get; init; }
    public ushort Energy { get; init; }
    public ushort Leadership { get; init; }
    public ushort LevelUpPoint { get; init; }

    public bool HasBaseStats =>
        this.Strength > 0 || this.Dexterity > 0 || this.Vitality > 0 || this.Energy > 0;

    public static CharacterSheetStats FromInts(
        int strength,
        int dexterity,
        int vitality,
        int energy,
        int leadership,
        int levelUpPoint) =>
        new()
        {
            Strength = (ushort)Math.Clamp(strength, 0, ushort.MaxValue),
            Dexterity = (ushort)Math.Clamp(dexterity, 0, ushort.MaxValue),
            Vitality = (ushort)Math.Clamp(vitality, 0, ushort.MaxValue),
            Energy = (ushort)Math.Clamp(energy, 0, ushort.MaxValue),
            Leadership = (ushort)Math.Clamp(leadership, 0, ushort.MaxValue),
            LevelUpPoint = (ushort)Math.Clamp(levelUpPoint, 0, ushort.MaxValue),
        };
}

/// <summary>Computed max/current vitals from stats (parity <c>CharacterCalcAttribute</c> simplified).</summary>
public readonly struct CharacterComputedVitals
{
    public ushort Life { get; init; }
    public ushort LifeMax { get; init; }
    public ushort Mana { get; init; }
    public ushort ManaMax { get; init; }
    public ushort Shield { get; init; }
    public ushort ShieldMax { get; init; }
    public ushort SkillMana { get; init; }
    public ushort SkillManaMax { get; init; }
}
