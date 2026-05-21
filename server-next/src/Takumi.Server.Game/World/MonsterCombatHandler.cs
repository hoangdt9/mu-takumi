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

            var hitRange = SkillCombatCatalog.GetSkillHitRange(targetedSkillId, isTargetedPacket: true);
            var useStat = TryRollPlayerSkillDamage(
                player,
                presenceSessionId,
                targetedSkillId,
                out var skillDmg,
                out var skillDmgType);

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
                    onRosterDirty,
                    skillDamageOverride: useStat ? skillDmg : null,
                    skillDamageTypeOverride: useStat ? skillDmgType : null,
                    hitRangeOverride: hitRange)
                .ConfigureAwait(false);

            Console.WriteLine(
                "[{0}] [m9] skill targeted id={1} target=0x{2:X4} range={3} statDmg={4}",
                remote,
                targetedSkillId,
                skillTargetId,
                hitRange,
                useStat ? skillDmg : -1);
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
                    magicSkillId,
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

        if (ClientHitPackets602.TryFindMagicContinue(
                packet,
                out _,
                out var continueSkillId,
                out var continueX,
                out var continueY,
                out var continueAngle,
                out _))
        {
            await HandleMagicContinueAsync(
                    tracker,
                    connection,
                    clientProtectOutbound,
                    mapId,
                    playerX,
                    playerY,
                    continueSkillId,
                    continueX,
                    continueY,
                    continueAngle,
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

    static async Task HandleMagicContinueAsync(
        MonsterViewportTracker tracker,
        Connection connection,
        (byte K1, byte K2)? clientProtectOutbound,
        byte mapId,
        byte playerX,
        byte playerY,
        ushort skillId,
        byte skillX,
        byte skillY,
        byte skillAngleWire,
        string remote,
        CancellationToken ct,
        int playerLevel,
        Guid? presenceSessionId,
        GameRosterEntry? player,
        string? accountId = null,
        Action? onRosterDirty = null)
    {
        if (IsPlayerCombatBlocked(presenceSessionId))
        {
            return;
        }

        if (!SkillCombatCatalog.IsAreaContinueSkill(skillId))
        {
            Console.WriteLine(
                "[{0}] [m9] magic continue skip — unsupported skill={1}",
                remote,
                skillId);
            return;
        }

        var directional = SkillCombatCatalog.IsDirectionalContinueSkill(skillId);
        var corridor = SkillCombatCatalog.IsForwardCorridorContinueSkill(skillId);
        (var centerX, var centerY) = SkillCombatRange.GetAreaContinueCenter(
            playerX,
            playerY,
            skillX,
            skillY,
            directional);
        var range = SkillCombatCatalog.GetAreaContinueRange(skillId);
        var facingWire = skillAngleWire;
        if ((directional || corridor) && facingWire == 0 && presenceSessionId is { } dirSid
            && GameMapPresenceRegistry.TryGetSession(dirSid, out var presence)
            && presence is not null)
        {
            facingWire = presence.Angle;
        }
        var wearSlots = CharacterCalcBroadcast602.ResolveWearSlots(presenceSessionId);
        var effects = PlayerCombatEffectSession.GetOrEmpty(presenceSessionId);
        var rolledSkillDamage = 0;
        var rolledSkillDamageType = (byte)0;
        var useStatDamage = player is not null
            && PlayerSkillCombatDamage602.TryRollWizardryHit(
                player.ServerClass,
                Math.Max((ushort)1, player.Level),
                player.ResolveSheet(),
                wearSlots,
                effects == CombatEffectState602.Empty ? null : effects,
                skillId,
                Random.Shared,
                out rolledSkillDamage,
                out rolledSkillDamageType);

        MapMonsterWorld.EnsureInitialized();
        var hitCount = 0;
        foreach (var mob in MapMonsterWorld.GetMonstersOnMap(mapId))
        {
            if (!mob.IsAlive || mob.IsNpc)
            {
                continue;
            }

            if (!SkillCombatRange.IsMobInSkillVolume(
                    skillId,
                    centerX,
                    centerY,
                    facingWire,
                    mob.X,
                    mob.Y,
                    range))
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
                    onRosterDirty,
                    skillDamageOverride: useStatDamage ? rolledSkillDamage : null,
                    skillDamageTypeOverride: useStatDamage ? rolledSkillDamageType : null)
                .ConfigureAwait(false);
            hitCount++;
        }

        var hitMode = corridor ? 2 : directional ? 1 : 0;
        Console.WriteLine(
            "[{0}] [m9] magic continue skill={1} center=({2},{3}) range={4} hits={5} mode={6} angle={7} statDmg={8}",
            remote,
            skillId,
            centerX,
            centerY,
            range,
            hitCount,
            hitMode,
            facingWire,
            useStatDamage ? rolledSkillDamage : -1);
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
        ushort skillId,
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

        var directionalBurst = SkillCombatCatalog.IsDirectionalContinueSkill(skillId);
        var aoeRange = directionalBurst
            ? SkillCombatCatalog.GetAreaContinueRange(skillId)
            : SkillCombatCatalog.IsMagicBurstSkill(skillId)
                ? SkillCombatCatalog.GetSkillHitRange(skillId, isTargetedPacket: false)
                : ParseIntEnv("TAKUMI_COMBAT_AOE_RANGE", 3, 1, 8);
        // Omnidirectional bursts (Hellfire, …) radiate from the caster, not the ground target tile.
        var burstOriginX = playerX;
        var burstOriginY = playerY;
        var facingWire = (byte)0;
        if (directionalBurst && presenceSessionId is { } burstSid
            && GameMapPresenceRegistry.TryGetSession(burstSid, out var burstPresence)
            && burstPresence is not null)
        {
            facingWire = burstPresence.Angle;
        }

        var skillPct = ParseIntEnv("TAKUMI_COMBAT_SKILL_DAMAGE_PCT", 150, 50, 500);
        var useStat = TryRollPlayerSkillDamage(
            player,
            presenceSessionId,
            skillId,
            out var burstDmg,
            out var burstDmgType);
        var processed = new HashSet<int>();

        foreach (var tid in targetIds)
        {
            var tk = tid & 0x7FFF;
            if (directionalBurst
                && MapMonsterWorld.TryResolveCombatTarget(mapId, playerX, playerY, tid, aoeRange, out var burstMob)
                && burstMob is not null
                && !SkillCombatDirection.IsInForwardArc(
                    burstOriginX,
                    burstOriginY,
                    facingWire,
                    burstMob.X,
                    burstMob.Y,
                    aoeRange))
            {
                continue;
            }

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
                    onRosterDirty,
                    skillDamageOverride: useStat ? burstDmg : null,
                    skillDamageTypeOverride: useStat ? burstDmgType : null,
                    hitRangeOverride: aoeRange)
                .ConfigureAwait(false);
            processed.Add(tid & 0x7FFF);
        }

        if (!directionalBurst)
        {
            MapMonsterWorld.EnsureInitialized();
            foreach (var mob in MapMonsterWorld.GetMonstersOnMap(mapId))
            {
                if (!mob.IsAlive || mob.IsNpc || processed.Contains(mob.ObjectKey))
                {
                    continue;
                }

                if (!SkillCombatRange.IsWithinChebyshev(burstOriginX, burstOriginY, mob.X, mob.Y, aoeRange))
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
                    onRosterDirty,
                    skillDamageOverride: useStat ? burstDmg : null,
                    skillDamageTypeOverride: useStat ? burstDmgType : null,
                    hitRangeOverride: aoeRange)
                    .ConfigureAwait(false);
            }
        }

        Console.WriteLine(
            "[{0}] [m9] magic aoe skill={1} xy=({2},{3}) targets={4} range={5} dir={6} statDmg={7} map={8}",
            remote,
            skillId,
            burstOriginX,
            burstOriginY,
            targetIds.Count,
            aoeRange,
            directionalBurst ? 1 : 0,
            useStat ? burstDmg : -1,
            mapId);
    }

    static bool TryRollPlayerSkillDamage(
        GameRosterEntry? player,
        Guid? presenceSessionId,
        ushort skillId,
        out int damage,
        out byte damageType)
    {
        damage = 0;
        damageType = 0;
        if (player is null || !SkillCombatCatalog.UsesMagicDamage(skillId))
        {
            return false;
        }

        var wearSlots = CharacterCalcBroadcast602.ResolveWearSlots(presenceSessionId);
        var effects = PlayerCombatEffectSession.GetOrEmpty(presenceSessionId);
        return PlayerSkillCombatDamage602.TryRollWizardryHit(
            player.ServerClass,
            Math.Max((ushort)1, player.Level),
            player.ResolveSheet(),
            wearSlots,
            effects == CombatEffectState602.Empty ? null : effects,
            skillId,
            Random.Shared,
            out damage,
            out damageType);
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
        Action? onRosterDirty = null,
        int? skillDamageOverride = null,
        byte? skillDamageTypeOverride = null,
        int? hitRangeOverride = null)
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
        var meleeRange = ParseIntEnv("TAKUMI_COMBAT_MELEE_RANGE", 3, 1, 15);
        if (!MapMonsterWorld.TryResolveCombatTarget(
                mapId,
                playerX,
                playerY,
                targetId,
                meleeRange,
                out var monster)
            || monster is null
            || !monster.IsAlive)
        {
            Console.WriteLine(
                "[{0}] [m9] combat skip — unknown/dead monster key={1} wireTarget=0x{2:X4} map={3} xy=({4},{5})",
                remote,
                targetKey,
                targetId,
                mapId,
                playerX,
                playerY);
            return;
        }

        if (monster.ObjectKey != targetKey)
        {
            Console.WriteLine(
                "[{0}] [m9] combat remap wireTarget=0x{1:X4} clientKey={2} -> serverKey={3} map={4} xy=({5},{6}) mob=({7},{8})",
                remote,
                targetId,
                targetKey,
                monster.ObjectKey,
                mapId,
                playerX,
                playerY,
                monster.X,
                monster.Y);
        }

        if (monster.IsNpc)
        {
            Console.WriteLine("[{0}] [m9] combat skip — target key={1} is NPC", remote, targetKey);
            return;
        }

        if (monster.Map != mapId)
        {
            Console.WriteLine(
                "[{0}] [m9] combat skip — key={1} on map {2}, player map {3}",
                remote,
                targetKey,
                monster.Map,
                mapId);
            return;
        }

        if (!skillDamageOverride.HasValue)
        {
            var dist = Math.Abs(monster.X - playerX) + Math.Abs(monster.Y - playerY);
            var maxRange = hitRangeOverride ?? meleeRange;
            if (dist > maxRange)
            {
                Console.WriteLine(
                    "[{0}] [m9] combat skip — key={1} out of range dist={2} max={3} player=({4},{5}) mob=({6},{7})",
                    remote,
                    targetKey,
                    dist,
                    maxRange,
                    playerX,
                    playerY,
                    monster.X,
                    monster.Y);
                return;
            }
        }
        else if (hitRangeOverride is int skillRange)
        {
            if (!SkillCombatRange.IsWithinChebyshev(playerX, playerY, monster.X, monster.Y, skillRange))
            {
                Console.WriteLine(
                    "[{0}] [m9] combat skip — key={1} out of skill range max={2} player=({3},{4}) mob=({5},{6})",
                    remote,
                    targetKey,
                    skillRange,
                    playerX,
                    playerY,
                    monster.X,
                    monster.Y);
                return;
            }
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
                stuckFlag: false);
            await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, missPkt, ct).ConfigureAwait(false);
            Console.WriteLine(
                "[{0}] [m9] combat miss key={1}",
                remote,
                monster.ObjectKey);
            return;
        }

        var stat = MapMonsterWorld.GetMonsterStat(monster.MonsterClass);
        var attackElement = MonsterCombatCalculator.ResolveAttackElement();
        int damage;
        byte damageType;
        if (skillDamageOverride is int skillDmg)
        {
            damage = MonsterCombatCalculator.ApplySkillDamageToMonster(skillDmg, attackElement, stat);
            damageType = skillDamageTypeOverride ?? 0;
        }
        else
        {
            var fallback = ParseIntEnv("TAKUMI_COMBAT_STUB_DAMAGE", 50, 1, 65_000);
            var skillPct = isSkill ? ParseIntEnv("TAKUMI_COMBAT_SKILL_DAMAGE_PCT", 150, 50, 500) : 100;
            damage = MonsterCombatCalculator.RollDamageToMonster(
                playerLevel,
                stat,
                fallback,
                skillPct,
                attackElement);
            damageType = MonsterCombatCalculator.RollClientDamageType(Random.Shared, isSkill);
            damage = MonsterCombatCalculator.ApplyClientDamageTypeMultiplier(damage, damageType);
        }

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
            stuckFlag: false,
            damageType: damageType);
        await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, dmgPkt, ct).ConfigureAwait(false);
        Console.WriteLine(
            "[{0}] [m9] combat {6} key={1} dmg={2} hp={3} died={4} statOverride={5} dmgType=0x{7:X2}",
            remote,
            monster.ObjectKey,
            damage,
            monster.CurrentLife,
            died,
            skillDamageOverride.HasValue ? 1 : 0,
            isSkill ? "skill" : "hit",
            damageType);

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
