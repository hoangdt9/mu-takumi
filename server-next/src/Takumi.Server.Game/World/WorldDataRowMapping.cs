using System.Text.Json;
using Takumi.Server.Persistence;

namespace Takumi.Server.Game.World;

public static class MapGateRowMapping
{
    public static MapGateRow ToRow(MapGateEntry e) =>
        new()
        {
            GateIndex = e.GateIndex,
            Flag = (short)e.Flag,
            MapId = e.MapId,
            PosX = e.PosX,
            PosY = e.PosY,
            RangeTx = e.RangeTx,
            RangeTy = e.RangeTy,
            TargetGate = e.TargetGate,
            Dir = e.Dir,
            MinLevel = e.MinLevel,
            MaxLevel = e.MaxLevel,
            MinReset = e.MinReset,
            MaxReset = e.MaxReset,
            AccountLevel = e.AccountLevel,
        };

    public static MapGateEntry FromRow(MapGateRow r) =>
        new()
        {
            GateIndex = r.GateIndex,
            Flag = r.Flag,
            MapId = r.MapId,
            PosX = r.PosX,
            PosY = r.PosY,
            RangeTx = r.RangeTx,
            RangeTy = r.RangeTy,
            TargetGate = r.TargetGate,
            Dir = r.Dir,
            MinLevel = r.MinLevel,
            MaxLevel = r.MaxLevel,
            MinReset = r.MinReset,
            MaxReset = r.MaxReset,
            AccountLevel = r.AccountLevel,
        };
}

public static class NpcShopRowMapping
{
    public static NpcShopRow ToRow(NpcShopEntry e) =>
        new()
        {
            ShopIndex = e.ShopIndex,
            MonsterClass = e.MonsterClass,
            MapId = e.MapId,
            PosX = e.PosX,
            PosY = e.PosY,
            Comment = e.Comment,
        };

    public static NpcShopEntry FromRow(NpcShopRow r) =>
        new()
        {
            ShopIndex = r.ShopIndex,
            MonsterClass = r.MonsterClass,
            MapId = r.MapId,
            PosX = r.PosX,
            PosY = r.PosY,
            Comment = r.Comment,
        };

    public static NpcShopItemRow ToRow(NpcShopItemEntry e) =>
        new()
        {
            ShopIndex = e.ShopIndex,
            Slot = e.Slot,
            ItemGroup = e.ItemGroup,
            ItemIndex = e.ItemIndex,
            ItemLevel = e.ItemLevel,
            Durability = e.Durability,
            Skill = e.Skill,
            Luck = e.Luck,
            Option = e.Option,
            ExcOpt = e.ExcOpt,
            Anc = e.Anc,
            Joh = e.Joh,
            Oex = e.Oex,
            Socket1 = e.Socket1,
            Socket2 = e.Socket2,
            Socket3 = e.Socket3,
            Socket4 = e.Socket4,
            Socket5 = e.Socket5,
            ItemName = e.ItemName,
        };

    public static NpcShopItemEntry FromRow(NpcShopItemRow r) =>
        new()
        {
            ShopIndex = r.ShopIndex,
            Slot = r.Slot,
            ItemGroup = r.ItemGroup,
            ItemIndex = r.ItemIndex,
            ItemLevel = r.ItemLevel,
            Durability = r.Durability,
            Skill = r.Skill,
            Luck = r.Luck,
            Option = r.Option,
            ExcOpt = r.ExcOpt,
            Anc = r.Anc,
            Joh = r.Joh,
            Oex = r.Oex,
            Socket1 = r.Socket1,
            Socket2 = r.Socket2,
            Socket3 = r.Socket3,
            Socket4 = r.Socket4,
            Socket5 = r.Socket5,
            ItemName = r.ItemName,
        };
}

public static class CustomWorldConfigBuilder
{
    public static CustomWorldConfigRow FromTableFile(string configKey, string path)
    {
        var rows = GameDataTextTableLoader.ReadDataRows(path).Select(cols => cols.ToArray()).ToList();
        var payload = JsonSerializer.Serialize(new { kind = "table", rows });
        return new CustomWorldConfigRow
        {
            ConfigKey = configKey,
            Format = "table",
            PayloadJson = payload,
        };
    }

    public static CustomWorldConfigRow FromRawFile(string configKey, string path, string format)
    {
        var text = File.ReadAllText(path);
        var payload = JsonSerializer.Serialize(new { kind = "raw", text });
        return new CustomWorldConfigRow
        {
            ConfigKey = configKey,
            Format = format,
            PayloadJson = payload,
        };
    }
}
