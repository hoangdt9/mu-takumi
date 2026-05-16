namespace Takumi.Server.Game.World;

/// <summary>Loads <c>Move/Move.txt</c> (parity <c>CMove::Load</c>).</summary>
public static class MoveLoader
{
    public static IReadOnlyList<MoveMapEntry> LoadFromFile(string path)
    {
        if (!File.Exists(path))
        {
            throw new FileNotFoundException("Move file not found.", path);
        }

        var list = new List<MoveMapEntry>(64);
        foreach (var parts in GameDataTextTableLoader.ReadDataRows(path))
        {
            if (parts.Length < 9)
            {
                continue;
            }

            var tail = parts.Length - 7;
            list.Add(
                new MoveMapEntry
                {
                    Index = GameDataTextTableLoader.ParseInt(parts[0]),
                    Money = GameDataTextTableLoader.ParseInt(parts[tail]),
                    MinLevel = GameDataTextTableLoader.ParseIntOrStar(parts[tail + 1]),
                    MaxLevel = GameDataTextTableLoader.ParseIntOrStar(parts[tail + 2]),
                    MinReset = GameDataTextTableLoader.ParseIntOrStar(parts[tail + 3]),
                    MaxReset = GameDataTextTableLoader.ParseIntOrStar(parts[tail + 4]),
                    AccountLevel = GameDataTextTableLoader.ParseIntOrStar(parts[tail + 5]),
                    Gate = GameDataTextTableLoader.ParseInt(parts[tail + 6]),
                });
        }

        return list;
    }
}
