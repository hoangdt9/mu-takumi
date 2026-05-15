namespace Takumi.Server.Game.World;

/// <summary>Loads <c>ShopManager.txt</c> (parity <c>CShopManager::Load</c>).</summary>
public static class ShopManagerLoader
{
    public static IReadOnlyList<NpcShopEntry> LoadFromFile(string path)
    {
        if (!File.Exists(path))
        {
            throw new FileNotFoundException("ShopManager file not found.", path);
        }

        var list = new List<NpcShopEntry>(64);
        foreach (var parts in GameDataTextTableLoader.ReadDataRows(path))
        {
            if (parts.Length < 5)
            {
                continue;
            }

            string? comment = null;
            if (parts.Length > 5)
            {
                comment = string.Join(' ', parts.Skip(5));
            }

            list.Add(
                new NpcShopEntry
                {
                    ShopIndex = GameDataTextTableLoader.ParseInt(parts[0]),
                    MonsterClass = GameDataTextTableLoader.ParseInt(parts[1]),
                    MapId = GameDataTextTableLoader.ParseShortOrStarNullable(parts[2]),
                    PosX = GameDataTextTableLoader.ParseShortOrStarNullable(parts[3]),
                    PosY = GameDataTextTableLoader.ParseShortOrStarNullable(parts[4]),
                    Comment = comment,
                });
        }

        return list;
    }
}
