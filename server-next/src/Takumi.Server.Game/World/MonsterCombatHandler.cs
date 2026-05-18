using System.Globalization;
using MUnique.OpenMU.Network;
using Takumi.Server.Game.Networking;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>M9 combat stub + M10 map broadcast: melee/skill vs monsters, damage, die, action to peers.</summary>
public static class MonsterCombatHandler
{
    public static async Task<bool> TryHandleCombatPacketAsync(
        MonsterViewportTracker tracker,
        Connection connection,
        (byte K1, byte K2)? clientProtectOutbound,
        byte mapId,
        byte playerX,
        byte playerY,
        byte[] packet,
        string remote,
        CancellationToken ct,
        int playerLevel = 1,
        Guid? presenceSessionId = null,
        GameRosterEntry? player = null,
        string? accountId = null,
        Action? onRosterDirty = null)
    {
        if (ClientHitPackets602.TryFindHitRequest(
                packet,
                out _,
                out var targetId,
                out var attackAnimation,
                out var lookingDirection))
        {
            await HandleHitAsync(
                    tracker,
                    connection,
                    clientProtectOutbound,
                    mapId,
                    playerX,
                    playerY,
                    targetId,
                    remote,
                    ct,
                    playerLevel,
                    isSkill: false,
                    attackAnimation,
                    lookingDirection,
                    presenceSessionId,
                    player,
                    accountId,
                    onRosterDirty)
                .ConfigureAwait(false);
            return true;
        }

        if (ClientHitPackets602.TryFindTargetedSkill(packet, out _, out var skillTargetId, out var targetedSkillId))
        {
            await TryRefreshCalcAfterBuffSkillAsync(
                    connection,
                    clientProtectOutbound,
                    player,
                    presenceSessionId,
                    targetedSkillId,
                    ct)
                .ConfigureAwait(false);

            await HandleHitAsync(
                    tracker,
                    connection,
                    clientProtectOutbound,
                    mapId,
                    playerX,
                    playerY,
                    skillTargetId,
                    remote,
                    ct,
                    playerLevel,
                    isSkill: true,
                    attackAnimation: 0,
                    lookingDirection: 0,
                    presenceSessionId,
                    player,
                    accountId,
                    onRosterDirty)
                .ConfigureAwait(false);
            return true;
        }

        if (ClientHitPackets602.TryFindMagicAttack(packet, out _, out var magicSkillId, out var skillX, out var skillY, out var magicTargets))
        {
            await TryRefreshCalcAfterBuffSkillAsync(
                    connection,
                    clientProtectOutbound,
                    player,
                    presenceSessionId,
                    magicSkillId,
                    ct)
                .ConfigureAwait(false);

            await HandleMagicAttackAsync(
                    tracker,
                    connection,
                    clientProtectOutbound,
                    mapId,
                    playerX,
                    playerY,
                    skillX,
                    skillY,
                    magicTargets,
                    remote,
                    ct,
                    playerLevel,
                    presenceSessionId,
                    player,
                    accountId,
                    onRosterDirty)
                .ConfigureAwait(false);
            return true;
        }

        return false;
    }

    static async Task HandleMagicAttackAsync(
        MonsterViewportTracker tracker,
        Connection connection,
        (byte K1, byte K2)? clientProtectOutbound,
        byte mapId,
        byte playerX,
        byte playerY,
        byte skillX,
        byte skillY,
        List<ushort> targetIds,
        string remote,
        CancellationToken ct,
        int playerLevel,
        Guid? presenceSessionId,
        GameRosterEntry? player = null,
        string? accountId = null,
        Action? onRosterDirty = null)
    {
        if (IsPlayerCombatBlocked(presenceSessionId))
        {
            return;
        }

        var aoeRange = ParseIntEnv("TAKUMI_COMBAT_AOE_RANGE", 3, 1, 8);
        var skillPct = ParseIntEnv("TAKUMI_COMBAT_SKILL_DAMAGE_PCT", 150, 50, 500);
        var processed = new HashSet<int>();

        foreach (var tid in targetIds)
        {
            var tk = tid & 0x7FFF;
            if (MonsterViewerRegistry.TryGetByPlayerKey(tk, out _))
            {
                await TryHandlePvPAsync(
                        tk,
                        mapId,
                        playerX,
                        playerY,
                        playerLevel,
                        skillPct,
                        presenceSessionId,
                        remote,
                        ct)
                    .ConfigureAwait(false);
                processed.Add(tk);
                continue;
            }

            await HandleHitAsync(
                    tracker,
                    connection,
                    clientProtectOutbound,
                    mapId,
                    playerX,
                    playerY,
                    tid,
                    remote,
                    ct,
                    playerLevel,
                    isSkill: true,
                    attackAnimation: 0,
                    lookingDirection: 0,
                    presenceSessionId,
                    player,
                    accountId,
                    onRosterDirty)
                .ConfigureAwait(false);
            processed.Add(tid & 0x7FFF);
        }

        MapMonsterWorld.EnsureInitialized();
        foreach (var mob in MapMonsterWorld.GetMonstersOnMap(mapId))
        {
            if (!mob.IsAlive || mob.IsNpc || processed.Contains(mob.ObjectKey))
            {
                continue;
            }

            if (Math.Abs(mob.X - skillX) + Math.Abs(mob.Y - skillY) > aoeRange)
            {
                continue;
            }

            await HandleHitAsync(
                    tracker,
                    connection,
                    clientProtectOutbound,
                    mapId,
                    playerX,
                    playerY,
                    (ushort)mob.ObjectKey,
                    remote,
                    ct,
                    playerLevel,
                    isSkill: true,
                    attackAnimation: 0,
                    lookingDirection: 0,
                    presenceSessionId,
                    player,
                    accountId,
                    onRosterDirty)
                .ConfigureAwait(false);
        }

        Console.WriteLine(
            "[{0}] [m9] magic aoe xy=({1},{2}) targets={3} map={4}",
            remote,
            skillX,
            skillY,
            targetIds.Count,
            mapId);
    }

    static async Task<bool> TryHandlePvPAsync(
        int targetPlayerKey,
        byte mapId,
        byte attackerX,
        byte attackerY,
        int attackerLevel,
        int skillPct,
        Guid? presenceSessionId,
        string remote,
        CancellationToken ct)
    {
        if (!PlayerCombatRules.IsPvPEnabled())
        {
            return false;
        }

        if (!MonsterViewerRegistry.TryGetByPlayerKey(targetPlayerKey, out var victim) || victim.MapId != mapId)
        {
            return false;
        }

        if (presenceSessionId is { } sid && GameMapPresenceRegistry.TryGetObjectKey(sid, out var atkKey) && atkKey == targetPlayerKey)
        {
            return false;
        }

        if (!PlayerCombatRules.CanAttackPlayer(mapId, attackerX, attackerY, victim.X, victim.Y))
        {
            return false;
        }

        var fallback = ParseIntEnv("TAKUMI_COMBAT_STUB_DAMAGE", 50, 1, 65_000);
        var victimLevel = Math.Max(1, (int)victim.PlayerLevel);
        var dmg = MonsterCombatCalculator.RollDamagePlayerToPlayer(
            attackerLevel,
            victimLevel,
            skillPct,
            fallback);

        var attackerPlayerKey = 0;
        if (presenceSessionId is { } psid && GameMapPresenceRegistry.TryGetObjectKey(psid, out var pk))
        {
            attackerPlayerKey = pk;
        }

        await MonsterViewerRegistry.ApplyPvPHitAsync(attackerPlayerKey, targetPlayerKey, mapId, dmg, ct).ConfigureAwait(false);
        Console.WriteLine(
            "[{0}] [m10c] pvp skillPct={1} dmg={2} target={3} atk={4}",
            remote,
            skillPct,
            dmg,
            targetPlayerKey,
            attackerPlayerKey);
        return true;
    }

    static bool IsPlayerCombatBlocked(Guid? presenceSessionId)
    {
        if (presenceSessionId is not { } sid)
        {
            return false;
        }

        if (PlayerVitalsState.IsDead(sid))
        {
            return true;
        }

        if (MonsterViewerRegistry.TryGetSession(sid, out var session) && session.CurrentHp <= 0)
        {
            return true;
        }

        return false;
    }

    static async Task HandleHitAsync(
        MonsterViewportTracker tracker,
        Connection connection,
        (byte K1, byte K2)? clientProtectOutbound,
        byte mapId,
        byte playerX,
        byte playerY,
        ushort targetId,
        string remote,
        CancellationToken ct,
        int playerLevel,
        bool isSkill,
        byte attackAnimation,
        byte lookingDirection,
        Guid? presenceSessionId,
        GameRosterEntry? player = null,
        string? accountId = null,
        Action? onRosterDirty = null)
    {
        if (IsPlayerCombatBlocked(presenceSessionId))
        {
            return;
        }

        var targetKey = targetId & 0x7FFF;
        if (await TryHandlePvPAsync(
                targetKey,
                mapId,
                playerX,
                playerY,
                playerLevel,
                skillPct: isSkill ? ParseIntEnv("TAKUMI_COMBAT_SKILL_DAMAGE_PCT", 150, 50, 500) : 100,
                presenceSessionId,
                remote,
                ct).ConfigureAwait(false))
        {
            return;
        }

        MapMonsterWorld.EnsureInitialized();
        if (!MapMonsterWorld.TryGetMonster(targetId, out var monster) || monster is null || !monster.IsAlive)
        {
            return;
        }

        if (monster.IsNpc)
        {
            return;
        }

        if (monster.Map != mapId)
        {
            return;
        }

        var meleeRange = ParseIntEnv("TAKUMI_COMBAT_MELEE_RANGE", 3, 1, 15);
        if (Math.Abs(monster.X - playerX) + Math.Abs(monster.Y - playerY) > meleeRange)
        {
            return;
        }

        if (presenceSessionId is { } sid)
        {
            var action = attackAnimation != 0 ? attackAnimation : (byte)(isSkill ? 0x02 : 0x00);
            var dir = lookingDirection;
            await GameMapPresenceRegistry.BroadcastActionAsync(
                    sid,
                    mapId,
                    dir,
                    action,
                    monster.ObjectKey,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        var missRate = ParseIntEnv("TAKUMI_COMBAT_MISS_RATE_PCT", 0, 0, 100);
        if (MonsterCombatCalculator.RollMiss(missRate))
        {
            var missPkt = MonsterDamageWire602.Build(
                monster.ObjectKey,
                damage: 0,
                monster.CurrentLife,
                hitSuccess: false);
            await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, missPkt, ct).ConfigureAwait(false);
            Console.WriteLine(
                "[{0}] [m9] combat miss key={1}",
                remote,
                monster.ObjectKey);
            return;
        }

        var stat = MapMonsterWorld.GetMonsterStat(monster.MonsterClass);
        var fallback = ParseIntEnv("TAKUMI_COMBAT_STUB_DAMAGE", 50, 1, 65_000);
        var skillPct = isSkill ? ParseIntEnv("TAKUMI_COMBAT_SKILL_DAMAGE_PCT", 150, 50, 500) : 100;
        var attackElement = MonsterCombatCalculator.ResolveAttackElement();
        var damage = MonsterCombatCalculator.RollDamageToMonster(
            playerLevel,
            stat,
            fallback,
            skillPct,
            attackElement);
        if (presenceSessionId is { } aggroSid && GameMapPresenceRegistry.TryGetObjectKey(aggroSid, out var playerKey))
        {
            monster.SetAggro(playerKey);
            monster.RecordHit(playerKey, damage);
        }

        var died = monster.ApplyDamage(damage);
        var dmgPkt = MonsterDamageWire602.Build(
            monster.ObjectKey,
            damage,
            monster.CurrentLife,
            hitSuccess: true);
        await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, dmgPkt, ct).ConfigureAwait(false);
        Console.WriteLine(
            "[{0}] [m9] combat {6} key={1} dmg={2} hp={3} died={4} skillPct={5}",
            remote,
            monster.ObjectKey,
            damage,
            monster.CurrentLife,
            died,
            skillPct,
            isSkill ? "skill" : "hit");

        if (!died)
        {
            return;
        }

        var expStub = ComputeKillExperience(monster);
        if (player is not null)
        {
            MonsterKillExperienceGrant.Grant(
                monster,
                expStub,
                player,
                accountId,
                presenceSessionId,
                onRosterDirty);
        }

        var diePkt = MonsterDieWire602.Build(monster.ObjectKey, expStub, damage, dieSuccess: true);
        await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, diePkt, ct).ConfigureAwait(false);

        tracker.Forget(monster.ObjectKey);
        await MonsterViewportBroadcast.BroadcastDestroyAsync(monster, ct).ConfigureAwait(false);
        var destroyPkt = MonsterViewportDestroyWire602.Build([monster.ObjectKey]);
        await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, destroyPkt, ct).ConfigureAwait(false);
        Console.WriteLine(
            "[{0}] [m9] monster died key={1} sent C1 0x16 die then C1 0x14 destroy exp={2}",
            remote,
            monster.ObjectKey,
            expStub);
    }

    static ushort ComputeKillExperience(MapMonsterInstance monster)
    {
        var baseExp = Math.Clamp(monster.Level * 10, 1, 5000);
        if (!monster.TryGetTopDamagePlayerKey(out _, out _))
        {
            return (ushort)baseExp;
        }

        var bonusPct = ParseIntEnv("TAKUMI_COMBAT_TOP_DAMAGE_EXP_BONUS_PCT", 0, 0, 200);
        if (bonusPct > 0)
        {
            baseExp = Math.Clamp(baseExp + (baseExp * bonusPct / 100), 1, 65_000);
        }

        return (ushort)Math.Clamp(baseExp, 1, ushort.MaxValue);
    }

    static int ParseIntEnv(string key, int defaultValue, int min, int max)
    {
        var raw = Environment.GetEnvironmentVariable(key);
        if (string.IsNullOrWhiteSpace(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var v))
        {
            return defaultValue;
        }

        return Math.Clamp(v, min, max);
    }

    static async Task TryRefreshCalcAfterBuffSkillAsync(
        Connection connection,
        (byte K1, byte K2)? clientProtectOutbound,
        GameRosterEntry? player,
        Guid? presenceSessionId,
        ushort skillId,
        CancellationToken ct)
    {
        if (player is null || presenceSessionId is null || !IsPreviewBuffSkill(skillId))
        {
            return;
        }

        var roster = player.ToWireWithSheet();
        var lv = Math.Max((ushort)1, roster.Level);
        var sheet = CharacterSheetCalculator.ResolveSheet(roster.ServerClass, lv, roster.Sheet);
        var acc = new CharacterCombatAccumulator();
        if (!PlayerCombatEffectSession.TryRegisterSelfBuff(presenceSessionId, skillId, roster.ServerClass, sheet, acc))
        {
            return;
        }

        var pkt = CharacterCalcBroadcast602.BuildCalcPacket(
            player,
            presenceSessionId,
            CharacterCalcBroadcast602.ResolveWearSlots(presenceSessionId));
        await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, pkt, ct).ConfigureAwait(false);
    }

    static bool IsPreviewBuffSkill(ushort skillId) =>
        skillId is SkillBuffPreview602.SkillGreaterDefense
            or SkillBuffPreview602.SkillGreaterDamage
            or SkillBuffPreview602.SkillGreaterLife
            or SkillBuffPreview602.SkillGreaterMana
            or SkillBuffPreview602.SkillGreaterCriticalDamage
            or SkillBuffPreview602.SkillSwordPower
            or SkillBuffPreview602.SkillMagicCircle;
}
