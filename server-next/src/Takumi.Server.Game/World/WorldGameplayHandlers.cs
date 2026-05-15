using MUnique.OpenMU.Network;
using Takumi.Server.Game.Networking;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>M8/M9 gameplay stubs: gate teleport (<c>0x1C</c>), NPC shop (<c>0x30</c> → <c>0x31</c>).</summary>
public static class WorldGameplayHandlers
{
    public static async Task<bool> TryHandlePacketAsync(
        GameRosterEntry player,
        MonsterViewportTracker tracker,
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        string? accountId,
        byte[] characterName10,
        Guid presenceSessionId,
        byte[] packet,
        string remote,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        CancellationToken ct)
    {
        if (ClientGameplayPackets602.TryFindShopExitRequest(packet, out _))
        {
            PlayerShopSession.CloseShop(presenceSessionId);
            Console.WriteLine("[m8] shop exit 0x31 from {0}", remote);
            return true;
        }

        if (await ItemWorldHandler.TryHandlePacketAsync(
                player,
                presenceSessionId,
                accountId,
                characterName10,
                packet,
                remote,
                writeAsync,
                onRosterDirty,
                ct).ConfigureAwait(false))
        {
            return true;
        }

        if (await ShopCommerceHandler.TryHandlePacketAsync(
                player,
                presenceSessionId,
                accountId,
                characterName10,
                packet,
                remote,
                writeAsync,
                onRosterDirty,
                ct).ConfigureAwait(false))
        {
            return true;
        }

        if (ClientGameplayPackets602.TryFindFinishLoadingRequest(packet, out _))
        {
            await RefreshViewportAfterRelocateAsync(
                    player,
                    tracker,
                    connection,
                    clientProtectOutbound,
                    remote,
                    presenceSessionId,
                    writeAsync,
                    ct)
                .ConfigureAwait(false);
            Console.WriteLine("[m8] F3 12 finish loading → viewport refresh {0}", remote);
            return true;
        }

        if (ClientGameplayPackets602.TryFindNpcTalkRequest(packet, out _, out var talkKey))
        {
            return await TryHandleNpcTalkAsync(
                    player,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    talkKey,
                    writeAsync,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        if (ClientGameplayPackets602.TryFindTeleportRequest(packet, out _, out var gate, out var tx, out var ty))
        {
            return await TryHandleTeleportAsync(
                    player,
                    tracker,
                    connection,
                    clientProtectOutbound,
                    accountId,
                    characterName10,
                    presenceSessionId,
                    gate,
                    tx,
                    ty,
                    writeAsync,
                    onRosterDirty,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        return false;
    }

    static async Task<bool> TryHandleTeleportAsync(
        GameRosterEntry player,
        MonsterViewportTracker tracker,
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        string? accountId,
        byte[] characterName10,
        Guid presenceSessionId,
        ushort gate,
        byte tx,
        byte ty,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        string remote,
        CancellationToken ct)
    {
        var prevMap = player.MapId;
        MapGateService.TeleportDestination dest;
        if (gate == 0)
        {
            if (!MapGateService.TryResolveSkillTeleport(player.MapId, tx, ty, player.Angle, prevMap, out dest))
            {
                return false;
            }
        }
        else if (!MapGateService.TryResolveGateTeleport(
                     gate,
                     player.MapId,
                     player.PosX,
                     player.PosY,
                     player.Level,
                     prevMap,
                     out dest))
        {
            var fail = TeleportWire602.Build(0, player.MapId, player.PosX, player.PosY, player.Angle);
            await writeAsync(fail, ct).ConfigureAwait(false);
            Console.WriteLine("[m8] gate {0} denied for {1} map={2} xy=({3},{4})", gate, remote, player.MapId, player.PosX, player.PosY);
            return true;
        }

        player.MapId = dest.MapId;
        player.PosX = dest.X;
        player.PosY = dest.Y;
        player.Angle = dest.Angle;
        onRosterDirty?.Invoke();

        var flag = (ushort)(dest.MapChanged ? 1 : 0);
        var tele = TeleportWire602.Build(flag, dest.MapId, dest.X, dest.Y, dest.Angle);
        await WriteGameplayAsync(connection, clientProtectOutbound, writeAsync, tele, ct).ConfigureAwait(false);

        if (dest.MapChanged && connection is not null && accountId is not null)
        {
            tracker.ResetForMap(dest.MapId, dest.X, dest.Y);
            var spawn = new JoinMapSpawnWire(dest.MapId, dest.X, dest.Y, dest.Angle);
            var joinPkt = JoinMapServerWire602.Build(ToWire(player), spawn);
            var invPkt = await JoinInventoryPacket602
                .BuildAsync(TakumiPostgresMirror.InventorySlots, accountId, characterName10, ct)
                .ConfigureAwait(false);
            await WriteGameplayAsync(connection, clientProtectOutbound, writeAsync, joinPkt, ct).ConfigureAwait(false);
            await WriteGameplayAsync(connection, clientProtectOutbound, writeAsync, invPkt, ct).ConfigureAwait(false);
            MonsterViewerRegistry.UpdatePosition(presenceSessionId, dest.MapId, dest.X, dest.Y);
            await MapMonsterScopeSender.TrySendAfterJoinAsync(
                    tracker,
                    connection,
                    clientProtectOutbound,
                    dest.MapId,
                    dest.X,
                    dest.Y,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        Console.WriteLine(
            "[m8] teleport gate={0} → map={1} xy=({2},{3}) flag={4} {5}",
            gate,
            dest.MapId,
            dest.X,
            dest.Y,
            flag,
            remote);
        return true;
    }

    static async Task<bool> TryHandleNpcTalkAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        int objectKey,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        string remote,
        CancellationToken ct)
    {
        var key = objectKey & 0x7FFF;
        if (!MapMonsterWorld.TryGetMonster(key, out var mob) || mob is null)
        {
            Console.WriteLine("[m8] npc talk unknown key={0} {1}", key, remote);
            return false;
        }

        if (!mob.IsNpc)
        {
            return false;
        }

        if (NpcTalkService.TryGetTalkResult(mob.MonsterClass, out var talkResult))
        {
            if (talkResult == NpcTalkService.TalkResultWarehouse)
            {
                await NpcTalkService.TryOpenWarehouseAsync(player, writeAsync, ct).ConfigureAwait(false);
                Console.WriteLine("[m8] warehouse open npc class={0} {1}", mob.MonsterClass, remote);
                return true;
            }

            if (talkResult == NpcTalkService.TalkResultDuelNpc)
            {
                await NpcTalkService.TryOpenDuelNpcAsync(writeAsync, ct).ConfigureAwait(false);
                Console.WriteLine("[m8] duel npc talk class={0} {1}", mob.MonsterClass, remote);
                return true;
            }
        }

        var shopIndex = NpcShopCatalog.ResolveShopIndex(mob.MonsterClass, mob.Map, mob.X, mob.Y);
        if (shopIndex < 0)
        {
            if (NpcQuestCatalog.IsQuestNpc(mob.MonsterClass))
            {
                return await TryHandleQuestNpcTalkAsync(writeAsync, mob.MonsterClass, remote, ct).ConfigureAwait(false);
            }

            Console.WriteLine(
                "[m8] npc talk class={0} map={1} xy=({2},{3}) unhandled {4}",
                mob.MonsterClass,
                mob.Map,
                mob.X,
                mob.Y,
                remote);
            return false;
        }

        await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
            .ConfigureAwait(false);

        var items = NpcShopCatalog.GetItems(shopIndex);
        var wireItems = new List<NpcShopWire602.ShopItemWire>(items.Count);
        foreach (var item in items)
        {
            var blob = new byte[ItemWire602.WireBytes];
            ItemWire602.WriteSeason6Item(
                blob,
                item.ItemGroup,
                item.ItemIndex,
                item.ItemLevel,
                item.Durability,
                item.Skill != 0,
                item.Luck != 0,
                item.Option,
                item.ExcOpt);
            wireItems.Add(new NpcShopWire602.ShopItemWire((byte)item.Slot, blob));
        }

        await writeAsync(NpcTalkWire602.Build(NpcTalkService.TalkResultShop), ct).ConfigureAwait(false);
        var pkt = NpcShopWire602.Build(wireItems);
        await writeAsync(pkt, ct).ConfigureAwait(false);
        PlayerShopSession.OpenShop(presenceSessionId, shopIndex);

        if (PlayerShopSession.TryGetSessionSlots(presenceSessionId, out var bagSlots))
        {
            var valuePkt = ShopItemValueSender.BuildForShop(items, bagSlots);
            if (valuePkt.Length > 0)
            {
                await writeAsync(valuePkt, ct).ConfigureAwait(false);
            }
        }

        Console.WriteLine("[m8] sent shop 0x31 index={0} count={1} npc={2} {3}", shopIndex, wireItems.Count, mob.MonsterClass, remote);
        return true;
    }

    static async Task<bool> TryHandleQuestNpcTalkAsync(
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        int monsterClass,
        string remote,
        CancellationToken ct)
    {
        var questIndex = NpcQuestCatalog.DefaultQuestIndexForClass(monsterClass);
        await writeAsync(QuestWire602.BuildQuestInfo(), ct).ConfigureAwait(false);
        await writeAsync(QuestWire602.BuildQuestState(questIndex, 0), ct).ConfigureAwait(false);
        await writeAsync(NpcTalkWire602.Build(1), ct).ConfigureAwait(false);
        Console.WriteLine("[m9] quest npc talk class={0} quest={1} {2}", monsterClass, questIndex, remote);
        return true;
    }

    static async Task RefreshViewportAfterRelocateAsync(
        GameRosterEntry player,
        MonsterViewportTracker tracker,
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        string remote,
        Guid presenceSessionId,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        CancellationToken ct)
    {
        if (connection is null)
        {
            return;
        }

        tracker.ResetForMap(player.MapId, player.PosX, player.PosY);
        MonsterViewerRegistry.UpdatePosition(presenceSessionId, player.MapId, player.PosX, player.PosY);
        await MapMonsterScopeSender.TrySendAfterJoinAsync(
                tracker,
                connection,
                clientProtectOutbound,
                player.MapId,
                player.PosX,
                player.PosY,
                remote,
                ct)
            .ConfigureAwait(false);
    }

    static async Task WriteGameplayAsync(
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        byte[] pkt,
        CancellationToken ct)
    {
        if (connection is not null && clientProtectOutbound is not null)
        {
            await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, pkt, ct).ConfigureAwait(false);
            return;
        }

        await writeAsync(pkt, ct).ConfigureAwait(false);
    }

    static CharacterRosterWire ToWire(GameRosterEntry e) =>
        new(e.Name10, e.ServerClass, e.Level, CharacterRosterVitals.FromInts(e.CurrentHp, e.MaxHp, e.CurrentMp, e.MaxMp, e.Zen));
}
