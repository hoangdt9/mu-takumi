namespace Takumi.Server.Game.World;

/// <summary>Loads <c>Move/Gate.txt</c> (parity <c>CGate::Load</c>).</summary>
public static class GateLoader
{
    public static IReadOnlyList<MapGateEntry> LoadFromFile(string path)
    {
        if (!File.Exists(path))
        {
            throw new FileNotFoundException("Gate file not found.", path);
        }

        var list = new List<MapGateEntry>(256);
        foreach (var parts in GameDataTextTableLoader.ReadDataRows(path))
        {
            if (parts.Length < 14)
            {
                continue;
            }

            list.Add(
                new MapGateEntry
                {
                    GateIndex = GameDataTextTableLoader.ParseInt(parts[0]),
                    Flag = GameDataTextTableLoader.ParseInt(parts[1]),
                    MapId = (byte)GameDataTextTableLoader.ParseInt(parts[2]),
                    PosX = (short)GameDataTextTableLoader.ParseInt(parts[3]),
                    PosY = (short)GameDataTextTableLoader.ParseInt(parts[4]),
                    RangeTx = (short)GameDataTextTableLoader.ParseInt(parts[5]),
                    RangeTy = (short)GameDataTextTableLoader.ParseInt(parts[6]),
                    TargetGate = GameDataTextTableLoader.ParseInt(parts[7]),
                    Dir = (short)GameDataTextTableLoader.ParseInt(parts[8]),
                    MinLevel = GameDataTextTableLoader.ParseIntOrStar(parts[9]),
                    MaxLevel = GameDataTextTableLoader.ParseIntOrStar(parts[10]),
                    MinReset = GameDataTextTableLoader.ParseIntOrStar(parts[11]),
                    MaxReset = GameDataTextTableLoader.ParseIntOrStar(parts[12]),
                    AccountLevel = (short)GameDataTextTableLoader.ParseInt(parts[13]),
                });
        }

        return list;
    }
}
