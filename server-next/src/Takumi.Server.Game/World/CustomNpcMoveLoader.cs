namespace Takumi.Server.Game.World;

/// <summary>Loads <c>Custom/CustomNpcMove.txt</c> (parity <c>CCustomNpcMove::Load</c>).</summary>
public static class CustomNpcMoveLoader
{
    public static IReadOnlyList<CustomNpcMoveEntry> LoadFromFile(string path)
    {
        if (!File.Exists(path))
        {
            throw new FileNotFoundException("CustomNpcMove file not found.", path);
        }

        var list = new List<CustomNpcMoveEntry>(32);
        foreach (var parts in GameDataTextTableLoader.ReadDataRows(path))
        {
            if (parts.Length < 16)
            {
                continue;
            }

            list.Add(
                new CustomNpcMoveEntry
                {
                    Index = GameDataTextTableLoader.ParseInt(parts[0]),
                    MonsterClass = GameDataTextTableLoader.ParseInt(parts[1]),
                    NpcMap = (byte)GameDataTextTableLoader.ParseInt(parts[2]),
                    NpcX = (byte)GameDataTextTableLoader.ParseInt(parts[3]),
                    NpcY = (byte)GameDataTextTableLoader.ParseInt(parts[4]),
                    DestinationMap = (byte)GameDataTextTableLoader.ParseInt(parts[5]),
                    DestinationX = (byte)GameDataTextTableLoader.ParseInt(parts[6]),
                    DestinationY = (byte)GameDataTextTableLoader.ParseInt(parts[7]),
                    MinLevel = GameDataTextTableLoader.ParseIntOrStar(parts[8]),
                    MaxLevel = GameDataTextTableLoader.ParseIntOrStar(parts[9]),
                    MinReset = GameDataTextTableLoader.ParseIntOrStar(parts[10]),
                    MaxReset = GameDataTextTableLoader.ParseIntOrStar(parts[11]),
                    MinMasterReset = GameDataTextTableLoader.ParseIntOrStar(parts[12]),
                    MaxMasterReset = GameDataTextTableLoader.ParseIntOrStar(parts[13]),
                    AccountLevel = GameDataTextTableLoader.ParseInt(parts[14]),
                    PkMove = GameDataTextTableLoader.ParseInt(parts[15]),
                });
        }

        return list;
    }
}
