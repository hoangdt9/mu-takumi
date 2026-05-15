using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Routes <c>0x30</c> NPC talk by monster class (parity <c>CNpcTalk::NpcTalk</c>).</summary>
public static class NpcTalkService
{
    public const byte TalkResultShop = 0;
    public const byte TalkResultWarehouse = 2;
    public const byte TalkResultDuelNpc = 33;

    public static bool TryGetTalkResult(int monsterClass, out byte result)
    {
        result = monsterClass switch
        {
            240 or 383 or 384 => TalkResultWarehouse,
            479 => TalkResultDuelNpc,
            _ => 255,
        };

        return result != 255;
    }

    public static async Task<bool> TryOpenWarehouseAsync(
        GameRosterEntry player,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        CancellationToken ct)
    {
        await writeAsync(NpcTalkWire602.Build(TalkResultWarehouse), ct).ConfigureAwait(false);
        await writeAsync(NpcShopWire602.Build(Array.Empty<NpcShopWire602.ShopItemWire>(), listType: 0), ct)
            .ConfigureAwait(false);
        var invMoney = (uint)Math.Clamp(player.Zen, 0, uint.MaxValue);
        await writeAsync(WarehouseWire602.BuildMoney(invMoney, warehouseMoney: 0), ct).ConfigureAwait(false);
        await writeAsync(WarehouseWire602.BuildState(0), ct).ConfigureAwait(false);
        return true;
    }

    public static async Task<bool> TryOpenDuelNpcAsync(
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        CancellationToken ct)
    {
        await writeAsync(NpcTalkWire602.Build(TalkResultDuelNpc), ct).ConfigureAwait(false);
        return true;
    }
}
