namespace Takumi.Server.Persistence;

public sealed class NpcShopRow
{
    public int ShopIndex { get; init; }

    public int MonsterClass { get; init; }

    public short? MapId { get; init; }

    public short? PosX { get; init; }

    public short? PosY { get; init; }

    public string? Comment { get; init; }
}

public sealed class NpcShopItemRow
{
    public int Id { get; init; }

    public int ShopIndex { get; init; }

    public short Slot { get; init; }

    public short ItemGroup { get; init; }

    public short ItemIndex { get; init; }

    public short ItemLevel { get; init; }

    public short Durability { get; init; }

    public short Skill { get; init; }

    public short Luck { get; init; }

    public short Option { get; init; }

    public short ExcOpt { get; init; }

    public short Anc { get; init; }

    public short Joh { get; init; }

    public short Oex { get; init; }

    public short Socket1 { get; init; }

    public short Socket2 { get; init; }

    public short Socket3 { get; init; }

    public short Socket4 { get; init; }

    public short Socket5 { get; init; }

    public string? ItemName { get; init; }
}
