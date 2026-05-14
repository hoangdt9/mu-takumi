namespace Takumi.Server.Persistence;

/// <summary>One equipped or bag slot row for <c>inventory_slot</c> (wire item = 12 bytes).</summary>
public sealed class InventorySlotRow
{
    public byte Slot { get; init; }

    public byte[] Item12 { get; init; } = Array.Empty<byte>();
}
