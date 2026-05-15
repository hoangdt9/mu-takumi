using MUnique.OpenMU.Network;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>M8: NPC talk (<c>C1 0x30</c>) → shop open + <c>C2 0x31</c> item list.</summary>
public static class NpcShopHandler
{
    const int TalkRangeTiles = 5;

    public static async Task<bool> TryHandleTalkAsync(
        byte[] packet,
        GameRosterEntry character,
        Connection connection,
        (byte K1, byte K2)? protect,
        string remote,
        bool verbose,
        CancellationToken ct)
    {
        if (!GamePacketFinders.TryFindNpcTalkRequest(packet, out var objectKey))
        {
            return false;
        }

        if (!MapMonsterWorld.TryGetMonster(objectKey, out var npc) || npc is null)
        {
            if (verbose)
            {
                Console.WriteLine("[{0}] [m8] npc talk key={1} — unknown object", remote, objectKey);
            }

            return true;
        }

        if (!npc.IsNpc)
        {
            if (verbose)
            {
                Console.WriteLine("[{0}] [m8] npc talk key={1} class={2} — not NPC", remote, objectKey, npc.MonsterClass);
            }

            return true;
        }

        if (npc.Map != character.MapId
            || !IsNear(character.PosX, character.PosY, npc.X, npc.Y))
        {
            if (verbose)
            {
                Console.WriteLine(
                    "[{0}] [m8] npc talk key={1} — out of range player=({2},{3}) npc=({4},{5})",
                    remote,
                    objectKey,
                    character.PosX,
                    character.PosY,
                    npc.X,
                    npc.Y);
            }

            return true;
        }

        var shopIndex = NpcShopCatalog.ResolveShopIndex(npc.MonsterClass, npc.Map, npc.X, npc.Y);
        if (shopIndex < 0)
        {
            if (verbose)
            {
                Console.WriteLine(
                    "[{0}] [m8] npc talk key={1} class={2} — no shop mapping",
                    remote,
                    objectKey,
                    npc.MonsterClass);
            }

            return true;
        }

        var catalogItems = NpcShopCatalog.GetItems(shopIndex);
        if (catalogItems.Count == 0)
        {
            if (verbose)
            {
                Console.WriteLine("[{0}] [m8] npc talk shop={1} — empty catalog", remote, shopIndex);
            }

            return true;
        }

        var wireItems = new List<ShopWireItem>(catalogItems.Count);
        foreach (var row in catalogItems)
        {
            var item12 = new byte[Season6ItemWire602.ItemWireBytes];
            Season6ItemWire602.EncodeShopItem(
                item12,
                row.ItemGroup,
                row.ItemIndex,
                row.ItemLevel,
                row.Durability,
                row.Skill,
                row.Luck,
                row.Option,
                row.ExcOpt,
                row.Anc,
                row.Joh,
                row.Oex,
                row.Socket1,
                row.Socket2,
                row.Socket3,
                row.Socket4,
                row.Socket5);
            wireItems.Add(new ShopWireItem((byte)row.Slot, item12));
        }

        var talkPkt = NpcTalkWire602.BuildShopOpen();
        var shopPkt = ShopItemListWire602.Build(wireItems);
        await GamePortOutboundWire.WriteAsync(connection, protect, talkPkt, ct).ConfigureAwait(false);
        await GamePortOutboundWire.WriteAsync(connection, protect, shopPkt, ct).ConfigureAwait(false);

        if (verbose)
        {
            Console.WriteLine(
                "[{0}] [m8] npc shop talk key={1} shop={2} items={3} class={4} wireLen={5}",
                remote,
                objectKey,
                shopIndex,
                wireItems.Count,
                npc.MonsterClass,
                shopPkt.Length);
        }

        return true;
    }

    public static bool TryHandleShopClose(byte[] packet)
    {
        if (!GamePacketFinders.TryFindShopCloseRequest(packet))
        {
            return false;
        }

        return true;
    }

    static bool IsNear(byte px, byte py, byte nx, byte ny) =>
        px >= nx - TalkRangeTiles
        && px <= nx + TalkRangeTiles
        && py >= ny - TalkRangeTiles
        && py <= ny + TalkRangeTiles;
}
