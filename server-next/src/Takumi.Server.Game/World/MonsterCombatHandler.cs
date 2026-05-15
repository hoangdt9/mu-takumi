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
        Guid? presenceSessionId = null)
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
                    presenceSessionId)
                .ConfigureAwait(false);
            return true;
        }

        if (ClientHitPackets602.TryFindTargetedSkill(packet, out _, out var skillTargetId))
        {
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
                    presenceSessionId)
                .ConfigureAwait(false);
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
        Guid? presenceSessionId)
    {
        MapMonsterWorld.EnsureInitialized();
        if (!MapMonsterWorld.TryGetMonster(targetId, out var monster) || monster is null || !monster.IsAlive)
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
        var damage = MonsterCombatCalculator.RollDamageToMonster(playerLevel, stat, fallback, skillPct);
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

        tracker.Forget(monster.ObjectKey);
        var destroyPkt = MonsterViewportDestroyWire602.Build([monster.ObjectKey]);
        await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, destroyPkt, ct).ConfigureAwait(false);
        var expStub = (ushort)Math.Clamp(monster.Level * 10, 1, 5000);
        var diePkt = MonsterDieWire602.Build(monster.ObjectKey, expStub, damage);
        await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, diePkt, ct).ConfigureAwait(false);
        Console.WriteLine(
            "[{0}] [m9] monster died key={1} sent C1 0x14 destroy + C1 0x16 die",
            remote,
            monster.ObjectKey);
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
}
