namespace Takumi.Server.Persistence;

/// <summary>Maps in-memory roster entries to <see cref="CharacterRosterRow"/> for JSON ↔ Postgres (M7 vitals included).</summary>
public static class CharacterRosterRowMapping
{
    public static CharacterRosterRow ToRow(
        string name,
        byte serverClass,
        ushort level,
        byte mapId,
        byte posX,
        byte posY,
        byte angle,
        int currentHp,
        int maxHp,
        int currentMp,
        int maxMp,
        long zen,
        int currentShield = 0,
        int maxShield = 0,
        int strength = 0,
        int dexterity = 0,
        int vitality = 0,
        int energy = 0,
        int leadership = 0,
        int levelUpPoint = 0,
        int currentBp = 0,
        int maxBp = 0) =>
        new()
        {
            Name = name,
            ServerClass = serverClass,
            Level = level,
            MapId = mapId,
            PosX = posX,
            PosY = posY,
            Angle = angle,
            CurrentHp = currentHp,
            MaxHp = maxHp,
            CurrentMp = currentMp,
            MaxMp = maxMp,
            Zen = zen,
            CurrentShield = currentShield,
            MaxShield = maxShield,
            Strength = strength,
            Dexterity = dexterity,
            Vitality = vitality,
            Energy = energy,
            Leadership = leadership,
            LevelUpPoint = levelUpPoint,
            CurrentBp = currentBp,
            MaxBp = maxBp,
        };
}
