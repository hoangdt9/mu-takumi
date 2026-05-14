namespace Takumi.Server.Game;

/// <summary>Roster mutations shared by <c>LegacyLoginHost</c> and <c>GamePortMinimalSession</c>.</summary>
public static class GameRosterMutations
{
    public static void UpsertNewCharacter(List<GameRosterEntry> roster, byte[] name10, byte serverClass, ushort level = 1)
    {
        var copy = new byte[10];
        name10.AsSpan(0, Math.Min(10, name10.Length)).CopyTo(copy);
        for (var i = roster.Count - 1; i >= 0; i--)
        {
            if (GameNameUtil.NameBytesEqual(roster[i].Name10, copy))
            {
                roster.RemoveAt(i);
            }
        }

        var sp = GameSpawnEnv.ReadNewCharacterSpawnDefaultsFromEnv();
        roster.Add(
            new GameRosterEntry
            {
                Name10 = copy,
                ServerClass = serverClass,
                Level = level,
                MapId = sp.Map,
                PosX = sp.PositionX,
                PosY = sp.PositionY,
                Angle = sp.Angle,
            });
    }

    public static int RemoveByName(List<GameRosterEntry> roster, byte[] name10) =>
        roster.RemoveAll(e => GameNameUtil.NameBytesEqual(e.Name10, name10));
}
