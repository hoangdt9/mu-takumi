namespace Takumi.Server.Protocol;

/// <summary>Season 6 item move storage flags (<c>PMSG_ITEM_MOVE_RECV</c> SourceFlag/TargetFlag).</summary>
public static class ItemStorageFlags602
{
    public const byte Inventory = 0;
    public const byte Trade = 1;
    public const byte Warehouse = 2;

    /// <summary>Main warehouse grid (parity <c>WAREHOUSE_SIZE</c> 240).</summary>
    public const byte MaxWarehouseSlot = 239;
}
