using System.Collections.Concurrent;
using System.Globalization;
using MUnique.OpenMU.Network;
using Takumi.Server.Protocol;
using Takumi.Server.Game.World;

namespace Takumi.Server.Game.Networking;

/// <summary>Tracks in-game TCP sessions for monster viewport sync, AI broadcasts, and player damage.</summary>
public sealed class MonsterViewerSession
{
    internal MonsterViewerSession(
        Guid sessionId,
        Connection connection,
        (byte K1, byte K2)? protect)
    {
        SessionId = sessionId;
        Connection = connection;
        Protect = protect;
    }

    public Guid SessionId { get; }
    public Connection Connection { get; }
    public (byte K1, byte K2)? Protect { get; }
    public byte MapId { get; set; }
    public byte X { get; set; }
    public byte Y { get; set; }
    public int PlayerObjectKey { get; set; }
    public int CurrentHp { get; set; }
    public int MaxHp { get; set; }
    public int CurrentMp { get; set; }
    public int MaxMp { get; set; }
    public string? AccountLogin { get; set; }
    public string? CharacterName { get; set; }
    public MonsterViewportTracker? ViewportTracker { get; set; }
    public Action<int, int>? OnVitalsChanged { get; set; }
}

public readonly record struct MonsterViewerTarget(
    Guid SessionId,
    int PlayerObjectKey,
    byte X,
    byte Y);

public static class MonsterViewerRegistry
{
    static readonly ConcurrentDictionary<Guid, MonsterViewerSession> Sessions = new();

    public static bool IsAiEnabled =>
        !string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_MONSTER_AI_ENABLED")?.Trim(),
            "0",
            StringComparison.OrdinalIgnoreCase);

    public static void Register(
        Guid sessionId,
        Connection connection,
        (byte K1, byte K2)? protect,
        byte mapId,
        byte x,
        byte y,
        MonsterViewportTracker? viewportTracker = null,
        int playerObjectKey = 0,
        int currentHp = 0,
        int maxHp = 0,
        int currentMp = 0,
        int maxMp = 0,
        string? accountLogin = null,
        string? characterName = null,
        Action<int, int>? onVitalsChanged = null)
    {
        if (Sessions.TryGetValue(sessionId, out var existing))
        {
            existing.MapId = mapId;
            existing.X = x;
            existing.Y = y;
            existing.ViewportTracker = viewportTracker ?? existing.ViewportTracker;
            if (playerObjectKey != 0)
            {
                existing.PlayerObjectKey = playerObjectKey;
            }

            if (maxHp > 0)
            {
                existing.MaxHp = maxHp;
            }

            if (currentHp > 0)
            {
                existing.CurrentHp = currentHp;
            }

            if (maxMp > 0)
            {
                existing.MaxMp = maxMp;
            }

            if (currentMp > 0)
            {
                existing.CurrentMp = currentMp;
            }

            if (!string.IsNullOrEmpty(accountLogin))
            {
                existing.AccountLogin = accountLogin;
            }

            if (!string.IsNullOrWhiteSpace(characterName))
            {
                existing.CharacterName = characterName;
            }

            existing.OnVitalsChanged = onVitalsChanged ?? existing.OnVitalsChanged;
            return;
        }

        Sessions[sessionId] = new MonsterViewerSession(sessionId, connection, protect)
        {
            MapId = mapId,
            X = x,
            Y = y,
            ViewportTracker = viewportTracker,
            PlayerObjectKey = playerObjectKey,
            CurrentHp = currentHp,
            MaxHp = maxHp,
            CurrentMp = currentMp,
            MaxMp = maxMp,
            AccountLogin = accountLogin,
            CharacterName = characterName,
            OnVitalsChanged = onVitalsChanged,
        };
    }

    public static void UpdatePosition(Guid sessionId, byte mapId, byte x, byte y)
    {
        if (Sessions.TryGetValue(sessionId, out var session))
        {
            session.MapId = mapId;
            session.X = x;
            session.Y = y;
        }
    }

    public static void Unregister(Guid sessionId)
    {
        Sessions.TryRemove(sessionId, out _);
        PlayerVitalsState.Unregister(sessionId);
    }

    public static bool TryGetSession(Guid sessionId, out MonsterViewerSession session) =>
        Sessions.TryGetValue(sessionId, out session!);

    public static bool TryGetByPlayerKey(int playerKey, out MonsterViewerSession session)
    {
        foreach (var s in Sessions.Values)
        {
            if (s.PlayerObjectKey == playerKey)
            {
                session = s;
                return true;
            }
        }

        session = null!;
        return false;
    }

    public static async Task ApplyPvPHitAsync(
        int attackerPlayerKey,
        int victimPlayerKey,
        byte mapId,
        int damage,
        CancellationToken ct)
    {
        if (!TryGetByPlayerKey(victimPlayerKey, out var victim) || victim.MapId != mapId)
        {
            return;
        }

        var maxHp = Math.Max(1, victim.MaxHp);
        var dmg = Math.Max(1, damage);
        victim.CurrentHp = Math.Max(0, victim.CurrentHp - dmg);
        victim.OnVitalsChanged?.Invoke(victim.CurrentHp, maxHp);

        var dmgPkt = MonsterDamageWire602.Build(victim.PlayerObjectKey, dmg, victim.CurrentHp, hitSuccess: true);
        await GamePortOutboundWire.WriteAsync(victim.Connection, victim.Protect, dmgPkt, ct).ConfigureAwait(false);
        var lifePkt = LifeManaWire602.BuildLife(LifeManaWire602.TypeCurrent, (ushort)Math.Min(victim.CurrentHp, ushort.MaxValue));
        await GamePortOutboundWire.WriteAsync(victim.Connection, victim.Protect, lifePkt, ct).ConfigureAwait(false);

        Console.WriteLine(
            "[m10c] pvp hit victim={0} dmg={1} hp={2}/{3} from={4}",
            victim.PlayerObjectKey,
            dmg,
            victim.CurrentHp,
            maxHp,
            attackerPlayerKey);
    }

    public static IReadOnlyCollection<MonsterViewerSession> GetAllSessions() => Sessions.Values.ToArray();

    public static bool TryFindNearestTarget(
        byte mapId,
        byte mx,
        byte my,
        int range,
        out MonsterViewerTarget target)
    {
        target = default;
        var best = int.MaxValue;
        foreach (var viewer in Sessions.Values)
        {
            if (viewer.MapId != mapId || viewer.PlayerObjectKey == 0)
            {
                continue;
            }

            var dist = Math.Abs(viewer.X - mx) + Math.Abs(viewer.Y - my);
            if (dist > range || dist >= best)
            {
                continue;
            }

            best = dist;
            target = new MonsterViewerTarget(viewer.SessionId, viewer.PlayerObjectKey, viewer.X, viewer.Y);
        }

        return target.SessionId != Guid.Empty;
    }

    public static bool TryFindNearestPlayerKey(byte mapId, byte mx, byte my, int range, out int playerObjectKey)
    {
        playerObjectKey = 0;
        if (!TryFindNearestTarget(mapId, mx, my, range, out var t))
        {
            return false;
        }

        playerObjectKey = t.PlayerObjectKey;
        return true;
    }

    public static async Task BroadcastPacketInViewAsync(
        byte mapId,
        byte mx,
        byte my,
        byte[] packet,
        CancellationToken ct)
    {
        var viewRange = ParseIntEnv("TAKUMI_MONSTER_VIEW_RANGE", 15, 1, 32);
        foreach (var viewer in Sessions.Values)
        {
            if (viewer.MapId != mapId)
            {
                continue;
            }

            if (Math.Abs(viewer.X - mx) + Math.Abs(viewer.Y - my) > viewRange)
            {
                continue;
            }

            await GamePortOutboundWire.WriteAsync(viewer.Connection, viewer.Protect, packet, ct).ConfigureAwait(false);
        }
    }

    public static async Task BroadcastWalkAsync(
        int monsterObjectKey,
        byte mapId,
        byte monsterX,
        byte monsterY,
        CancellationToken ct)
    {
        if (!IsAiEnabled)
        {
            return;
        }

        var pkt = MonsterWalkWire602.Build(monsterObjectKey, monsterX, monsterY);
        await BroadcastPacketInViewAsync(mapId, monsterX, monsterY, pkt, ct).ConfigureAwait(false);
    }

    public static async Task BroadcastMonsterActionAsync(
        int monsterObjectKey,
        byte mapId,
        byte monsterX,
        byte monsterY,
        byte dir,
        byte action,
        int targetObjectKey,
        CancellationToken ct)
    {
        if (!IsAiEnabled)
        {
            return;
        }

        var pkt = PlayerActionWire602.Build(monsterObjectKey, dir, action, targetObjectKey);
        await BroadcastPacketInViewAsync(mapId, monsterX, monsterY, pkt, ct).ConfigureAwait(false);
    }

    public static async Task ApplyMonsterHitToPlayerAsync(
        Guid targetSessionId,
        int monsterObjectKey,
        byte mapId,
        byte monsterX,
        byte monsterY,
        int damagePercent,
        CancellationToken ct)
    {
        if (!Sessions.TryGetValue(targetSessionId, out var session) || session.PlayerObjectKey == 0)
        {
            return;
        }

        if (PlayerVitalsState.IsDead(targetSessionId))
        {
            return;
        }

        var baseDmg = ParseIntEnv("TAKUMI_MONSTER_TO_PLAYER_DAMAGE", 15, 1, 2000);
        var dmg = Math.Max(1, baseDmg * Math.Clamp(damagePercent, 50, 500) / 100);
        var maxHp = Math.Max(1, session.MaxHp);
        if (session.CurrentHp <= 0)
        {
            session.CurrentHp = maxHp;
        }

        session.CurrentHp = Math.Max(0, session.CurrentHp - dmg);
        session.OnVitalsChanged?.Invoke(session.CurrentHp, maxHp);
        RosterVitalsCombat.ScheduleVitalsMirror(
            session.AccountLogin,
            session.CharacterName,
            session.CurrentHp,
            maxHp,
            session.CurrentMp,
            session.MaxMp);

        var dmgPkt = MonsterDamageWire602.Build(
            session.PlayerObjectKey,
            dmg,
            session.CurrentHp,
            hitSuccess: true);
        await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, dmgPkt, ct).ConfigureAwait(false);

        var lifePkt = LifeManaWire602.BuildLife(
            LifeManaWire602.TypeCurrent,
            (ushort)Math.Clamp(session.CurrentHp, 0, ushort.MaxValue));
        await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, lifePkt, ct).ConfigureAwait(false);

        if (session.CurrentHp <= 0)
        {
            PlayerVitalsState.MarkDead(targetSessionId, PlayerVitalsState.ReviveDelayFromEnv());
            var diePkt = PlayerDieWire602.Build(session.PlayerObjectKey, monsterObjectKey);
            await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, diePkt, ct).ConfigureAwait(false);
        }

        Console.WriteLine(
            "[m9-ai] monster hit player key={0} dmg={1} hp={2}/{3} died={4} from mob={5}",
            session.PlayerObjectKey,
            dmg,
            session.CurrentHp,
            maxHp,
            session.CurrentHp <= 0,
            monsterObjectKey);
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
