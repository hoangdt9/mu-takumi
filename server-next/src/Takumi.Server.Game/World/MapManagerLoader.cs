namespace Takumi.Server.Game.World;

/// <summary>Loads <c>MapManager.txt</c> (parity <c>CMapManager::Load</c>).</summary>
public static class MapManagerLoader
{
    public static IReadOnlyList<MapManagerEntry> LoadFromFile(string path)
    {
        if (!File.Exists(path))
        {
            throw new FileNotFoundException("MapManager file not found.", path);
        }

        var list = new List<MapManagerEntry>(64);
        foreach (var parts in GameDataTextTableLoader.ReadDataRows(path))
        {
            if (parts.Length < 10 || !byte.TryParse(parts[0], out var mapId))
            {
                continue;
            }

            list.Add(
                new MapManagerEntry
                {
                    MapId = mapId,
                    GensBattle = GameDataTextTableLoader.ParseIntOrStar(parts[9]) is var g and >= 0 ? g : 0,
                });
        }

        return list;
    }
}
