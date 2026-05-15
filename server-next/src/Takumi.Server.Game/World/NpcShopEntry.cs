namespace Takumi.Server.Game.World;

/// <summary>One row from <c>ShopManager.txt</c> (parity <c>SHOP_MANAGER_INFO</c>).</summary>
public sealed class NpcShopEntry
{
    public int ShopIndex { get; init; }

    public int MonsterClass { get; init; }

    public short? MapId { get; init; }

    public short? PosX { get; init; }

    public short? PosY { get; init; }

    public string? Comment { get; init; }
}

/// <summary>One item line from <c>Shop/NNN - *.txt</c> (parity <c>CShop::Load</c>).</summary>
public sealed class NpcShopItemEntry
{
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

    public short Socket1 { get; init; } = 255;

    public short Socket2 { get; init; } = 255;

    public short Socket3 { get; init; } = 255;

    public short Socket4 { get; init; } = 255;

    public short Socket5 { get; init; } = 255;

    public string? ItemName { get; init; }
}
