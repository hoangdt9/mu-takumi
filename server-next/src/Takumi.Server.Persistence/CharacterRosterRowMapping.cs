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
        int maxShield = 0) =>
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
        };
}
