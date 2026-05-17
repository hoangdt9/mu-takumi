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

    /// <summary>Wire key from <c>C1 F1 00</c> index (client <c>HeroKey</c>). Differs from <see cref="PlayerObjectKey"/> used for viewport/presence.</summary>
    public int ClientHeroWireKey { get; set; }
    public int CurrentHp { get; set; }
    public int MaxHp { get; set; }
    public int CurrentMp { get; set; }
    public int MaxMp { get; set; }
    public int CurrentShield { get; set; }
    public int MaxShield { get; set; }
    public string? AccountLogin { get; set; }
    public string? CharacterName { get; set; }
    public ushort PlayerLevel { get; set; } = 1;
    public ulong Experience { get; set; }
    public uint Gold { get; set; }
    public byte ServerClass { get; set; }
    public CharacterSheetStats Sheet { get; set; }
    public MonsterViewportTracker? ViewportTracker { get; set; }
    public Action<int, int>? OnVitalsChanged { get; set; }

    public Action<int, int>? OnShieldVitalsChanged { get; set; }

    public Action<byte, byte, byte>? OnRosterPositionChanged { get; set; }

    /// <summary>Last HP sent on outbound 0x26 (throttle duplicate life packets under AI burst).</summary>
    internal int LastOutboundLifeHp { get; set; } = -1;

    internal int LastOutboundLifeSd { get; set; } = -1;

    internal long LastOutboundLifeUtcTicks { get; set; }
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
        int clientHeroWireKey = 0,
        int currentHp = 0,
        int maxHp = 0,
        int currentMp = 0,
        int maxMp = 0,
        int currentShield = 0,
        int maxShield = 0,
        string? accountLogin = null,
        string? characterName = null,
        ushort playerLevel = 1,
        ulong experience = 0,
        uint gold = 0,
        byte serverClass = 0,
        CharacterSheetStats sheet = default,
        Action<int, int>? onVitalsChanged = null,
        Action<int, int>? onShieldVitalsChanged = null,
        Action<byte, byte, byte>? onRosterPositionChanged = null)
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

            existing.ClientHeroWireKey = clientHeroWireKey;

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

            if (maxShield > 0)
            {
                existing.MaxShield = maxShield;
                existing.CurrentShield = currentShield > 0 ? currentShield : (existing.CurrentShield > 0 ? existing.CurrentShield : maxShield);
            }
            else if (currentShield > 0)
            {
                existing.CurrentShield = currentShield;
            }

            if (!string.IsNullOrEmpty(accountLogin))
            {
                existing.AccountLogin = accountLogin;
            }

            if (!string.IsNullOrWhiteSpace(characterName))
            {
                existing.CharacterName = characterName;
            }

            if (playerLevel > 0)
            {
                existing.PlayerLevel = playerLevel;
            }

            if (experience > 0)
            {
                existing.Experience = experience;
            }

            if (gold > 0)
            {
                existing.Gold = gold;
            }

            if (serverClass != 0)
            {
                existing.ServerClass = serverClass;
            }

            if (sheet.HasBaseStats)
            {
                existing.Sheet = sheet;
            }

            existing.OnVitalsChanged = onVitalsChanged ?? existing.OnVitalsChanged;
            existing.OnShieldVitalsChanged = onShieldVitalsChanged ?? existing.OnShieldVitalsChanged;
            existing.OnRosterPositionChanged = onRosterPositionChanged ?? existing.OnRosterPositionChanged;
            return;
        }

        var initSdMax = Math.Max(0, maxShield);
        var initSd = initSdMax > 0 ? (currentShield > 0 ? currentShield : initSdMax) : currentShield;
        Sessions[sessionId] = new MonsterViewerSession(sessionId, connection, protect)
        {
            MapId = mapId,
            X = x,
            Y = y,
            ViewportTracker = viewportTracker,
            PlayerObjectKey = playerObjectKey,
            ClientHeroWireKey = clientHeroWireKey,
            CurrentHp = currentHp,
            MaxHp = maxHp,
            CurrentMp = currentMp,
            MaxMp = maxMp,
            CurrentShield = initSd,
            MaxShield = initSdMax,
            AccountLogin = accountLogin,
            CharacterName = characterName,
            PlayerLevel = playerLevel > (ushort)0 ? playerLevel : (ushort)1,
            Experience = experience,
            Gold = gold,
            ServerClass = serverClass,
            Sheet = sheet,
            OnVitalsChanged = onVitalsChanged,
            OnShieldVitalsChanged = onShieldVitalsChanged,
            OnRosterPositionChanged = onRosterPositionChanged,
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

    public static void TryUpdatePlayerLevel(Guid sessionId, ushort playerLevel)
    {
        if (playerLevel == 0)
        {
            return;
        }

        if (Sessions.TryGetValue(sessionId, out var session))
        {
            session.PlayerLevel = playerLevel;
        }
    }

    public static void TryUpdateProgress(Guid sessionId, ulong experience, ushort playerLevel, uint gold)
    {
        if (!Sessions.TryGetValue(sessionId, out var session))
        {
            return;
        }

        session.Experience = experience;
        if (playerLevel > 0)
        {
            session.PlayerLevel = playerLevel;
        }

        if (gold > 0)
        {
            session.Gold = gold;
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
        var maxSd = Math.Max(0, victim.MaxShield);
        var sd = Math.Max(0, victim.CurrentShield);

        var dmg = Math.Max(1, damage);
        var remaining = dmg;
        var sdTaken = 0;
        if (maxSd > 0)
        {
            sdTaken = Math.Min(sd, remaining);
            sd -= sdTaken;
            remaining -= sdTaken;
        }

        victim.CurrentShield = sd;
        victim.CurrentHp = Math.Max(0, victim.CurrentHp - remaining);
        victim.OnVitalsChanged?.Invoke(victim.CurrentHp, maxHp);
        victim.OnShieldVitalsChanged?.Invoke(victim.CurrentShield, maxSd);
        RosterVitalsCombat.ScheduleVitalsMirror(
            victim.AccountLogin,
            victim.CharacterName,
            victim.CurrentHp,
            maxHp,
            victim.CurrentMp,
            victim.MaxMp,
            victim.CurrentShield,
            maxSd);

        var dmgPkt = MonsterDamageWire602.Build(
            victim.ClientHeroWireKey,
            remaining,
            victim.CurrentHp,
            hitSuccess: true,
            viewCurSd: victim.CurrentShield,
            shieldDamage: sdTaken);
        await GamePortOutboundWire.WriteAsync(victim.Connection, victim.Protect, dmgPkt, ct).ConfigureAwait(false);
        await TrySendThrottledLifeAsync(victim, ct).ConfigureAwait(false);

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
        int monsterClass,
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

        if (PlayerVitalsState.IsDead(targetSessionId) || session.CurrentHp <= 0)
        {
            return;
        }

        var fallbackStub = ParseIntEnv("TAKUMI_MONSTER_TO_PLAYER_DAMAGE", 15, 1, 2000);
        var useTxt = !string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_MONSTER_IGNORE_TXT_DAMAGE")?.Trim(),
            "1",
            StringComparison.OrdinalIgnoreCase);
        var stat = MapMonsterWorld.GetMonsterStat(monsterClass);
        var playerDef = session.Sheet.HasBaseStats
            ? CharacterCombatPreview602.ResolvePlayerDefense(session.ServerClass, session.PlayerLevel, session.Sheet)
            : MonsterCombatCalculator.ResolveStubPlayerDefense(session.PlayerLevel);
        var dmg = useTxt
            ? MonsterCombatCalculator.RollDamageFromMonsterToPlayer(
                stat,
                playerDef,
                damagePercent,
                Random.Shared,
                fallbackStub)
            : Math.Max(1, fallbackStub * Math.Clamp(damagePercent, 50, 500) / 100);
        var maxHp = Math.Max(1, session.MaxHp);
        var maxSd = Math.Max(0, session.MaxShield);
        var sd = Math.Max(0, session.CurrentShield);

        var remaining = dmg;
        var sdTaken = 0;
        if (maxSd > 0)
        {
            sdTaken = Math.Min(sd, remaining);
            sd -= sdTaken;
            remaining -= sdTaken;
        }

        session.CurrentShield = sd;
        session.CurrentHp = Math.Max(0, session.CurrentHp - remaining);
        session.OnVitalsChanged?.Invoke(session.CurrentHp, maxHp);
        session.OnShieldVitalsChanged?.Invoke(session.CurrentShield, maxSd);
        RosterVitalsCombat.ScheduleVitalsMirror(
            session.AccountLogin,
            session.CharacterName,
            session.CurrentHp,
            maxHp,
            session.CurrentMp,
            session.MaxMp,
            session.CurrentShield,
            maxSd);

        var dmgPkt = MonsterDamageWire602.Build(
            session.ClientHeroWireKey,
            remaining,
            session.CurrentHp,
            hitSuccess: true,
            viewCurSd: session.CurrentShield,
            shieldDamage: sdTaken);
        await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, dmgPkt, ct).ConfigureAwait(false);
        await TrySendThrottledLifeAsync(session, ct).ConfigureAwait(false);

        if (session.CurrentHp <= 0)
        {
            if (PlayerVitalsState.TryMarkDead(targetSessionId, PlayerVitalsState.ReviveDelayFromEnv()))
            {
                var diePkt = PlayerDieWire602.Build(session.ClientHeroWireKey, monsterObjectKey);
                await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, diePkt, ct).ConfigureAwait(false);
                PlayerVitalsLoop.ScheduleReviveAfterDeath(session);
            }
        }

        Console.WriteLine(
            "[m9-ai] monster hit player key={0} dmg={1} hpDmg={2} sdDmg={3} hp={4}/{5} sd={6}/{7} died={8} mobKey={9} mobClass={10} txt={11} def={12}",
            session.PlayerObjectKey,
            dmg,
            remaining,
            sdTaken,
            session.CurrentHp,
            maxHp,
            session.CurrentShield,
            maxSd,
            session.CurrentHp <= 0,
            monsterObjectKey,
            monsterClass,
            useTxt,
            playerDef);
    }

    static async Task TrySendThrottledLifeAsync(MonsterViewerSession session, CancellationToken ct)
    {
        var hp = Math.Clamp(session.CurrentHp, 0, ushort.MaxValue);
        var sdWire = (ushort)Math.Clamp(session.CurrentShield, 0, ushort.MaxValue);
        var now = Environment.TickCount64;
        var minIntervalMs = ParseIntEnv("TAKUMI_PLAYER_LIFE_PACKET_MIN_MS", 200, 50, 3000);
        var hpDropped = session.LastOutboundLifeHp < 0 || hp < session.LastOutboundLifeHp;
        var sdDropped = session.LastOutboundLifeSd < 0 || sdWire < session.LastOutboundLifeSd;
        if (!hpDropped
            && !sdDropped
            && session.LastOutboundLifeHp == hp
            && session.LastOutboundLifeSd == sdWire
            && (now - session.LastOutboundLifeUtcTicks) < minIntervalMs)
        {
            return;
        }

        session.LastOutboundLifeHp = hp;
        session.LastOutboundLifeSd = sdWire;
        session.LastOutboundLifeUtcTicks = now;
        var lifePkt = LifeManaWire602.BuildLife(
            LifeManaWire602.TypeCurrent,
            (ushort)hp,
            sdWire);
        await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, lifePkt, ct).ConfigureAwait(false);
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
