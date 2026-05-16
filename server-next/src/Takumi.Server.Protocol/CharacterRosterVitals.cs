namespace Takumi.Server.Protocol;

/// <summary>Persisted HP/MP/zen for join wire (M7). All zero = unset → <see cref="JoinMapServerWire602"/> uses class/level stub.</summary>
public readonly struct CharacterRosterVitals
{
    public static CharacterRosterVitals Unset => default;

    public int CurrentHp { get; init; }

    public int MaxHp { get; init; }

    public int CurrentMp { get; init; }

    public int MaxMp { get; init; }

    public long Zen { get; init; }

    /// <summary>SD / shield (legacy <c>GCLifeSend</c> second word).</summary>
    public int CurrentShield { get; init; }

    public int MaxShield { get; init; }

    public bool HasHp => this.MaxHp > 0;

    public bool HasMp => this.MaxMp > 0;

    /// <summary>Persisted max shield &gt; 0 — join + life wire use SD bar.</summary>
    public bool HasShield => this.MaxShield > 0;

    public static CharacterRosterVitals FromInts(
        int currentHp,
        int maxHp,
        int currentMp,
        int maxMp,
        long zen,
        int currentShield = 0,
        int maxShield = 0) =>
        new()
        {
            CurrentHp = currentHp,
            MaxHp = maxHp,
            CurrentMp = currentMp,
            MaxMp = maxMp,
            Zen = zen,
            CurrentShield = currentShield,
            MaxShield = maxShield,
        };

    internal ushort ClampU16(int value) => (ushort)Math.Clamp(value, 0, ushort.MaxValue);

    internal uint ClampGold() => this.Zen <= 0 ? 0u : (uint)Math.Min(this.Zen, uint.MaxValue);
}
