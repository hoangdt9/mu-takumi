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
        Action? onRosterSave,
        CancellationToken ct)
    {
        if (await CharacterStatPointHandler.TryHandleAsync(
                player,
                accountId,
                packet,
                writeAsync,
                onRosterDirty,
                ct).ConfigureAwait(false))
        {
            return true;
        }

        if (await PersonalShopGameplayHandler.TryHandlePacketAsync(
                presenceSessionId,
                packet,
                remote,
                writeAsync,
                ct).ConfigureAwait(false))
        {
            return true;
        }

        if (await MoveMapHandler.TryHandleAsync(
                player,
                tracker,
                connection,
                clientProtectOutbound,
                accountId,
                characterName10,
                presenceSessionId,
                packet,
                remote,
                writeAsync,
                onRosterDirty,
                onRosterSave,
                ct).ConfigureAwait(false))
        {
            return true;
        }

        if (ClientGameplayPackets602.TryFindShopExitRequest(packet, out _))
        {
            PlayerShopSession.CloseShop(presenceSessionId);
            PlayerWarehouseSession.Close(presenceSessionId);
            PlayerUiSession.SetPersonalShop(presenceSessionId, false);
            Console.WriteLine("[m8] shop exit 0x31 from {0}", remote);
            return true;
        }

        if (await TradeGameplayHandler.TryHandlePacketAsync(
                presenceSessionId,
                characterName10,
                packet,
                remote,
                writeAsync,
                ct).ConfigureAwait(false))
        {
            return true;
        }

        if (await GuildGameplayHandler.TryHandlePacketAsync(packet, remote, writeAsync, ct).ConfigureAwait(false))
        {
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

        if (await WarehouseGameplayHandler.TryHandlePacketAsync(
                player,
                presenceSessionId,
                accountId,
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
                    tracker,
                    connection,
                    clientProtectOutbound,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    talkKey,
                    writeAsync,
                    onRosterDirty,
                    onRosterSave,
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
        if (gate == 0)
        {
            return await TryHandleSkillTeleportAsync(
                    player,
                    tracker,
                    connection,
                    clientProtectOutbound,
                    presenceSessionId,
                    tx,
                    ty,
                    writeAsync,
                    onRosterDirty,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        MoveMapSessionState.SetTeleportInProgress(presenceSessionId, true);
        try
        {
            var prevMap = player.MapId;
            if (!MapGateService.TryResolveGateTeleport(
                    gate,
                    player.MapId,
                    player.PosX,
                    player.PosY,
                    player.Level,
                    player.Reset,
                    player.AccountLevel,
                    prevMap,
                    out var dest))
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

            if (dest.MapChanged)
            {
                var tele = TeleportWire602.Build(1, dest.MapId, dest.X, dest.Y, dest.Angle);
                await WriteGameplayAsync(connection, clientProtectOutbound, writeAsync, tele, ct).ConfigureAwait(false);
            }

            if (accountId is not null)
            {
                await MoveWarpJoinReload.SendAsync(
                        player,
                        tracker,
                        connection,
                        clientProtectOutbound,
                        accountId,
                        characterName10,
                        presenceSessionId,
                        dest,
                        writeAsync,
                        remote,
                        ct)
                    .ConfigureAwait(false);
            }

            await MoveMapPostWarp.SendPersonalShopViewportRedrawAsync(
                    connection,
                    clientProtectOutbound,
                    writeAsync,
                    ct)
                .ConfigureAwait(false);

            Console.WriteLine(
                "[m8] teleport gate={0} -> map={1} xy=({2},{3}) flag={4} {5}",
                gate,
                dest.MapId,
                dest.X,
                dest.Y,
                dest.MapChanged ? 1 : 0,
                remote);
            return true;
        }
        finally
        {
            MoveMapSessionState.SetTeleportInProgress(presenceSessionId, false);
        }
    }

    static async Task<bool> TryHandleSkillTeleportAsync(
        GameRosterEntry player,
        MonsterViewportTracker tracker,
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        Guid presenceSessionId,
        byte tx,
        byte ty,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        string remote,
        CancellationToken ct)
    {
        if (!SkillTeleportService.PlayerHasTeleportSkill(player.ServerClass))
        {
            return false;
        }

        if (!MapGateService.TryResolveSkillTeleport(
                player.MapId,
                player.PosX,
                player.PosY,
                tx,
                ty,
                player.Angle,
                out var dest))
        {
            var snap = TeleportWire602.Build(0, player.MapId, player.PosX, player.PosY, player.Angle);
            await WriteGameplayAsync(connection, clientProtectOutbound, writeAsync, snap, ct).ConfigureAwait(false);
            Console.WriteLine("[m8] skill teleport denied area {0} map={1} ({2},{3})->({4},{5})", remote, player.MapId, player.PosX, player.PosY, tx, ty);
            return true;
        }

        if (!SkillTeleportService.TryConsumeResources(player, out var manaSpent))
        {
            return false;
        }

        player.PosX = dest.X;
        player.PosY = dest.Y;
        onRosterDirty?.Invoke();

        var ok = TeleportWire602.Build(0, dest.MapId, dest.X, dest.Y, dest.Angle);
        await WriteGameplayAsync(connection, clientProtectOutbound, writeAsync, ok, ct).ConfigureAwait(false);

        var mp = Math.Clamp(player.CurrentMp, 0, ushort.MaxValue);
        var bp = Math.Clamp(player.CurrentBp, 0, ushort.MaxValue);
        var manaPkt = LifeManaWire602.BuildMana(LifeManaWire602.TypeCurrent, (ushort)mp, (ushort)bp);
        await WriteGameplayAsync(connection, clientProtectOutbound, writeAsync, manaPkt, ct).ConfigureAwait(false);

        MonsterViewerRegistry.UpdatePosition(presenceSessionId, dest.MapId, dest.X, dest.Y);
        if (connection is not null)
        {
            await MapMonsterScopeSender.TrySendOnMoveAsync(
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
            "[m8] skill teleport map={0} ({1},{2}) mana={3} {4}",
            dest.MapId,
            dest.X,
            dest.Y,
            manaSpent,
            remote);
        return true;
    }

    static async Task<bool> TryHandleNpcTalkAsync(
        GameRosterEntry player,
        MonsterViewportTracker tracker,
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        int objectKey,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        Action? onRosterSave,
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

        if (await CustomNpcMoveHandler.TryHandleNpcTalkWarpAsync(
                player,
                tracker,
                connection,
                clientProtectOutbound,
                accountId,
                characterName10,
                presenceSessionId,
                mob.MonsterClass,
                mob.Map,
                mob.X,
                mob.Y,
                writeAsync,
                onRosterDirty,
                onRosterSave,
                remote,
                ct)
            .ConfigureAwait(false))
        {
            return true;
        }

        if (NpcTalkService.TryGetTalkResult(mob.MonsterClass, out var talkResult))
        {
            if (talkResult == NpcTalkService.TalkResultWarehouse)
            {
                await NpcTalkService.TryOpenWarehouseAsync(
                        player,
                        presenceSessionId,
                        accountId,
                        writeAsync,
                        ct)
                    .ConfigureAwait(false);
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
            ShopItemWireEncoding.WriteShopEntry(blob, item);
            wireItems.Add(new NpcShopWire602.ShopItemWire((byte)item.Slot, blob));
        }

        await writeAsync(NpcTalkWire602.Build(NpcTalkService.TalkResultShop), ct).ConfigureAwait(false);
        var pkt = NpcShopWire602.Build(wireItems);
        await writeAsync(pkt, ct).ConfigureAwait(false);
        PlayerShopSession.OpenShop(presenceSessionId, shopIndex);

        var taxRate = PlayerShopSession.GetTaxRatePercent(presenceSessionId);
        await writeAsync(NpcShopTaxWire602.Build((byte)taxRate), ct).ConfigureAwait(false);

        var valuePkt = ShopItemValueSender.BuildForShop(items, taxRate);
        if (valuePkt.Length > 0)
        {
            await writeAsync(valuePkt, ct).ConfigureAwait(false);
        }

        Console.WriteLine("[m8] sent shop 0x31 index={0} count={1} npc={2} tax={3}% {4}", shopIndex, wireItems.Count, mob.MonsterClass, taxRate, remote);
        return true;
    }

    static async Task<bool> TryHandleQuestNpcTalkAsync(
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        int monsterClass,
        string remote,
        CancellationToken ct)
    {
        var questIndex = NpcQuestCatalog.DefaultQuestIndexForClass(monsterClass);
        var questState = NpcQuestCatalog.DefaultQuestStateForClass(monsterClass);
        await writeAsync(QuestWire602.BuildQuestInfo(NpcQuestCatalog.BuildQuestInfoMask(questIndex)), ct).ConfigureAwait(false);
        await writeAsync(QuestWire602.BuildQuestState(questIndex, questState), ct).ConfigureAwait(false);
        await writeAsync(NpcTalkWire602.Build(1), ct).ConfigureAwait(false);
        Console.WriteLine(
            "[m9] quest npc talk class={0} quest={1} state={2} {3}",
            monsterClass,
            questIndex,
            questState,
            remote);
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
}
