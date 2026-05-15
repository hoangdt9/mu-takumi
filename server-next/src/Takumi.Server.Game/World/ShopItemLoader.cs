using System.Text.RegularExpressions;

namespace Takumi.Server.Game.World;

/// <summary>Loads <c>Shop/NNN - Name.txt</c> inventories (parity <c>CShop::Load</c>).</summary>
public static partial class ShopItemLoader
{
    [GeneratedRegex(@"^(\d{3})\s+-\s+", RegexOptions.CultureInvariant)]
    private static partial Regex ShopFileIndexRegex();

    public static IReadOnlyList<NpcShopItemEntry> LoadFromFile(int shopIndex, string path)
    {
        if (!File.Exists(path))
        {
            throw new FileNotFoundException("Shop item file not found.", path);
        }

        var list = new List<NpcShopItemEntry>(32);
        short slot = 0;
        foreach (var parts in GameDataTextTableLoader.ReadDataRows(path))
        {
            if (parts.Length < 16)
            {
                continue;
            }

            string? itemName = null;
            if (parts.Length > 16)
            {
                itemName = string.Join(' ', parts.Skip(16));
            }

            list.Add(
                new NpcShopItemEntry
                {
                    ShopIndex = shopIndex,
                    Slot = slot++,
                    ItemGroup = (short)GameDataTextTableLoader.ParseInt(parts[0]),
                    ItemIndex = (short)GameDataTextTableLoader.ParseInt(parts[1]),
                    ItemLevel = (short)GameDataTextTableLoader.ParseInt(parts[2]),
                    Durability = (short)GameDataTextTableLoader.ParseInt(parts[3]),
                    Skill = (short)GameDataTextTableLoader.ParseInt(parts[4]),
                    Luck = (short)GameDataTextTableLoader.ParseInt(parts[5]),
                    Option = (short)GameDataTextTableLoader.ParseInt(parts[6]),
                    ExcOpt = (short)GameDataTextTableLoader.ParseInt(parts[7]),
                    Anc = (short)GameDataTextTableLoader.ParseInt(parts[8]),
                    Joh = (short)GameDataTextTableLoader.ParseInt(parts[9]),
                    Oex = (short)GameDataTextTableLoader.ParseInt(parts[10]),
                    Socket1 = (short)GameDataTextTableLoader.ParseInt(parts[11]),
                    Socket2 = (short)GameDataTextTableLoader.ParseInt(parts[12]),
                    Socket3 = (short)GameDataTextTableLoader.ParseInt(parts[13]),
                    Socket4 = (short)GameDataTextTableLoader.ParseInt(parts[14]),
                    Socket5 = (short)GameDataTextTableLoader.ParseInt(parts[15]),
                    ItemName = itemName,
                });
        }

        return list;
    }

    public static IReadOnlyList<NpcShopItemEntry> LoadAllForDataRoot(string dataRoot, IReadOnlyList<NpcShopEntry> shops)
    {
        var shopDir = Path.Combine(dataRoot, "Shop");
        if (!Directory.Exists(shopDir))
        {
            return Array.Empty<NpcShopItemEntry>();
        }

        var byIndex = shops.ToDictionary(s => s.ShopIndex);
        var all = new List<NpcShopItemEntry>(512);
        foreach (var file in Directory.EnumerateFiles(shopDir, "*.txt"))
        {
            var name = Path.GetFileName(file);
            var m = ShopFileIndexRegex().Match(name);
            if (!m.Success)
            {
                continue;
            }

            var index = int.Parse(m.Groups[1].Value, System.Globalization.CultureInfo.InvariantCulture);
            if (!byIndex.ContainsKey(index))
            {
                continue;
            }

            all.AddRange(LoadFromFile(index, file));
        }

        return all;
    }
}
