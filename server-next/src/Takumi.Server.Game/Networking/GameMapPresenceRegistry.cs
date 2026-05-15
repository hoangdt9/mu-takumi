using System.Collections.Concurrent;
using System.Globalization;
using MUnique.OpenMU.Network;
using Takumi.Server.Game;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.Networking;

/// <summary>M10: other players on the same map receive <c>C1 0x15</c> position and <c>C1 0x18</c> combat action.</summary>
public sealed class MapPresenceSession
{
    internal MapPresenceSession(
        Guid sessionId,
        Connection connection,
        (byte K1, byte K2)? protect,
        int objectKey)
    {
        SessionId = sessionId;
        Connection = connection;
        Protect = protect;
        ObjectKey = objectKey;
    }

    public Guid SessionId { get; }
    public Connection Connection { get; }
    public (byte K1, byte K2)? Protect { get; }
    public int ObjectKey { get; }
    public byte MapId { get; set; }
    public byte X { get; set; }
    public byte Y { get; set; }
    public byte Angle { get; set; }
    public PlayerPresenceAppearance Appearance { get; set; } = new();
}

public static class GameMapPresenceRegistry
{
    static readonly ConcurrentDictionary<Guid, MapPresenceSession> Sessions = new();
    static readonly ConcurrentDictionary<Guid, PlayerViewportTracker> ViewportTrackers = new();
    static readonly ConcurrentDictionary<Guid, DecryptedPacketRateGate> BroadcastGates = new();
    static int _nextPlayerKey = 1000;
    static readonly int MaxBroadcastsPerSecond = DecryptedPacketSafety602.ParseMaxPresenceBroadcastsPerSecond();

    public static bool IsEnabled =>
        !string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_MAP_PRESENCE_ENABLED")?.Trim(),
            "0",
            StringComparison.OrdinalIgnoreCase);

    public static bool UsePlayerViewportWire =>
        IsEnabled &&
        !string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_PLAYER_VIEWPORT_WIRE")?.Trim(),
            "0",
            StringComparison.OrdinalIgnoreCase);

    public static MapPresenceSession? Register(
        Guid sessionId,
        Connection connection,
        (byte K1, byte K2)? protect,
        byte mapId,
        byte x,
        byte y,
        byte angle,
        PlayerPresenceAppearance? appearance = null)
    {
        if (!IsEnabled)
        {
            return null;
        }

        if (Sessions.TryGetValue(sessionId, out var existing))
        {
            existing.MapId = mapId;
            existing.X = x;
            existing.Y = y;
            existing.Angle = angle;
            return existing;
        }

        var key = Interlocked.Increment(ref _nextPlayerKey);
        if (key >= 12_000)
        {
            Interlocked.Exchange(ref _nextPlayerKey, 1000);
            key = 1000;
        }

        var session = new MapPresenceSession(sessionId, connection, protect, key)
        {
            MapId = mapId,
            X = x,
            Y = y,
            Angle = angle,
            Appearance = appearance ?? new PlayerPresenceAppearance(),
        };
        Sessions[sessionId] = session;
        return session;
    }

    public static async Task UnregisterAsync(Guid sessionId, CancellationToken ct = default)
    {
        if (!Sessions.TryRemove(sessionId, out var leaving))
        {
            BroadcastGates.TryRemove(sessionId, out _);
            ViewportTrackers.TryRemove(sessionId, out _);
            return;
        }

        ViewportTrackers.TryRemove(sessionId, out _);
        BroadcastGates.TryRemove(sessionId, out _);

        if (!UsePlayerViewportWire)
        {
            return;
        }

        var destroy = MonsterViewportDestroyWire602.Build([leaving.ObjectKey]);
        foreach (var other in Sessions.Values)
        {
            if (other.MapId != leaving.MapId)
            {
                continue;
            }

            ViewportTrackers.GetOrAdd(other.SessionId, _ => new PlayerViewportTracker()).TryForget(leaving.ObjectKey);
            await GamePortOutboundWire.WriteAsync(other.Connection, other.Protect, destroy, ct).ConfigureAwait(false);
        }
    }

    public static void Unregister(Guid sessionId) =>
        UnregisterAsync(sessionId).GetAwaiter().GetResult();

    public static void UpdateMap(Guid sessionId, byte mapId, byte x, byte y, byte angle)
    {
        if (!Sessions.TryGetValue(sessionId, out var session))
        {
            return;
        }

        session.MapId = mapId;
        session.X = x;
        session.Y = y;
        session.Angle = angle;
    }

    public static bool TryGetObjectKey(Guid sessionId, out int objectKey)
    {
        if (Sessions.TryGetValue(sessionId, out var session))
        {
            objectKey = session.ObjectKey;
            return true;
        }

        objectKey = 0;
        return false;
    }

    public static async Task NotifyJoinAsync(MapPresenceSession self, string remote, CancellationToken ct)
    {
        var peerCount = 0;
        foreach (var other in Sessions.Values)
        {
            if (other.SessionId == self.SessionId || other.MapId != self.MapId)
            {
                continue;
            }

            peerCount++;
            if (UsePlayerViewportWire)
            {
                var otherVp = PlayerViewportWire602.Build([ToViewportEntry(other)]);
                await GamePortOutboundWire.WriteAsync(self.Connection, self.Protect, otherVp, ct).ConfigureAwait(false);
                var selfVp = PlayerViewportWire602.Build([ToViewportEntry(self)]);
                await GamePortOutboundWire.WriteAsync(other.Connection, other.Protect, selfVp, ct).ConfigureAwait(false);
            }

            var otherPos = PlayerPositionWire602.Build(other.ObjectKey, other.X, other.Y);
            await GamePortOutboundWire.WriteAsync(self.Connection, self.Protect, otherPos, ct).ConfigureAwait(false);
            var selfPos = PlayerPositionWire602.Build(self.ObjectKey, self.X, self.Y);
            await GamePortOutboundWire.WriteAsync(other.Connection, other.Protect, selfPos, ct).ConfigureAwait(false);
        }

        if (UsePlayerViewportWire)
        {
            var tracker = ViewportTrackers.GetOrAdd(self.SessionId, _ => new PlayerViewportTracker());
            tracker.ResetForMap(self.MapId, self.X, self.Y);
            var peerKeys = new List<int>();
            foreach (var other in Sessions.Values)
            {
                if (other.SessionId == self.SessionId || other.MapId != self.MapId)
                {
                    continue;
                }

                peerKeys.Add(other.ObjectKey);
                ViewportTrackers.GetOrAdd(other.SessionId, _ => new PlayerViewportTracker()).TryMarkVisible(self.ObjectKey);
            }

            tracker.SyncPeers(peerKeys);
        }

        Console.WriteLine(
            "[{0}] [m10] map presence join map={1} key={2} peers={3} viewport0x12={4}",
            remote,
            self.MapId,
            self.ObjectKey,
            peerCount,
            UsePlayerViewportWire);
    }

    static PlayerViewportEntry ToViewportEntry(MapPresenceSession s) =>
        new(
            s.ObjectKey,
            s.X,
            s.Y,
            s.X,
            s.Y,
            s.Appearance.ServerClass,
            s.Appearance.Name10,
            s.Angle,
            s.Appearance.PkLevel,
            CreateFlag: true);

    public static async Task BroadcastPositionAsync(
        Guid sessionId,
        byte mapId,
        byte x,
        byte y,
        string remote,
        CancellationToken ct)
    {
        if (!IsEnabled || !Sessions.TryGetValue(sessionId, out var self))
        {
            return;
        }

        self.MapId = mapId;
        self.X = x;
        self.Y = y;

        await TrySyncPlayerViewportOnMoveAsync(self, remote, ct).ConfigureAwait(false);

        if (!TryAllowBroadcast(sessionId))
        {
            return;
        }

        var pkt = PlayerPositionWire602.Build(self.ObjectKey, x, y);
        var sent = 0;
        foreach (var other in Sessions.Values)
        {
            if (other.SessionId == sessionId || other.MapId != mapId)
            {
                continue;
            }

            sent++;
            await GamePortOutboundWire.WriteAsync(other.Connection, other.Protect, pkt, ct).ConfigureAwait(false);
        }

        if (sent > 0)
        {
            Console.WriteLine(
                "[{0}] [m10] broadcast C1 0x15 key={1} xy=({2},{3}) map={4} peers={5}",
                remote,
                self.ObjectKey,
                x,
                y,
                mapId,
                sent);
        }
    }

    public static async Task TrySyncPlayerViewportOnMoveAsync(
        MapPresenceSession self,
        string remote,
        CancellationToken ct)
    {
        if (!UsePlayerViewportWire)
        {
            return;
        }

        var moveThreshold = ParseIntEnv("TAKUMI_PLAYER_VIEWPORT_MOVE_TILES", 4, 1, 16);
        var viewRange = ParseIntEnv("TAKUMI_PLAYER_VIEW_RANGE", 15, 1, 32);
        var tracker = ViewportTrackers.GetOrAdd(self.SessionId, _ => new PlayerViewportTracker());
        if (!tracker.ShouldRescan(self.MapId, self.X, self.Y, moveThreshold))
        {
            return;
        }

        var peersInRange = new List<MapPresenceSession>();
        foreach (var other in Sessions.Values)
        {
            if (other.SessionId == self.SessionId || other.MapId != self.MapId)
            {
                continue;
            }

            if (Manhattan(self.X, self.Y, other.X, other.Y) <= viewRange)
            {
                peersInRange.Add(other);
            }
        }

        var peerKeys = peersInRange.ConvertAll(p => p.ObjectKey);
        var (entered, left) = tracker.SyncPeers(peerKeys);
        tracker.NoteAnchor(self.MapId, self.X, self.Y);

        if (entered.Count > 0)
        {
            var entries = peersInRange
                .Where(p => entered.Contains(p.ObjectKey))
                .Select(ToViewportEntry)
                .ToList();
            var pkt = PlayerViewportWire602.Build(entries);
            await GamePortOutboundWire.WriteAsync(self.Connection, self.Protect, pkt, ct).ConfigureAwait(false);
        }

        if (left.Count > 0)
        {
            var destroy = MonsterViewportDestroyWire602.Build(left);
            await GamePortOutboundWire.WriteAsync(self.Connection, self.Protect, destroy, ct).ConfigureAwait(false);
        }

        var selfEntry = ToViewportEntry(self);
        foreach (var other in peersInRange)
        {
            var otherTracker = ViewportTrackers.GetOrAdd(other.SessionId, _ => new PlayerViewportTracker());
            if (entered.Contains(other.ObjectKey))
            {
                if (otherTracker.TryMarkVisible(self.ObjectKey))
                {
                    var pkt = PlayerViewportWire602.Build([selfEntry]);
                    await GamePortOutboundWire.WriteAsync(other.Connection, other.Protect, pkt, ct).ConfigureAwait(false);
                }
            }
        }

        foreach (var other in Sessions.Values)
        {
            if (other.SessionId == self.SessionId || other.MapId != self.MapId)
            {
                continue;
            }

            if (Manhattan(self.X, self.Y, other.X, other.Y) > viewRange)
            {
                var otherTracker = ViewportTrackers.GetOrAdd(other.SessionId, _ => new PlayerViewportTracker());
                if (otherTracker.TryForget(self.ObjectKey))
                {
                    var destroy = MonsterViewportDestroyWire602.Build([self.ObjectKey]);
                    await GamePortOutboundWire.WriteAsync(other.Connection, other.Protect, destroy, ct).ConfigureAwait(false);
                }
            }
        }

        if (entered.Count > 0 || left.Count > 0)
        {
            Console.WriteLine(
                "[{0}] [m10] player viewport walk map={1} key={2} +{3} -{4}",
                remote,
                self.MapId,
                self.ObjectKey,
                entered.Count,
                left.Count);
        }
    }

    static int Manhattan(byte x1, byte y1, byte x2, byte y2) =>
        Math.Abs(x1 - x2) + Math.Abs(y1 - y2);

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

    public static async Task BroadcastActionAsync(
        Guid sessionId,
        byte mapId,
        byte dir,
        byte action,
        int targetKey,
        string remote,
        CancellationToken ct)
    {
        if (!IsEnabled || !Sessions.TryGetValue(sessionId, out var self))
        {
            return;
        }

        if (!TryAllowBroadcast(sessionId))
        {
            return;
        }

        var pkt = PlayerActionWire602.Build(self.ObjectKey, dir, action, targetKey);
        var sent = 0;
        foreach (var other in Sessions.Values)
        {
            if (other.SessionId == sessionId || other.MapId != mapId)
            {
                continue;
            }

            sent++;
            await GamePortOutboundWire.WriteAsync(other.Connection, other.Protect, pkt, ct).ConfigureAwait(false);
        }

        if (sent > 0)
        {
            Console.WriteLine(
                "[{0}] [m10] broadcast C1 0x18 key={1} action={2} target={3} peers={4}",
                remote,
                self.ObjectKey,
                action,
                targetKey,
                sent);
        }
    }

    static bool TryAllowBroadcast(Guid sessionId)
    {
        if (MaxBroadcastsPerSecond <= 0)
        {
            return true;
        }

        var gate = BroadcastGates.GetOrAdd(sessionId, _ => new DecryptedPacketRateGate(MaxBroadcastsPerSecond));
        return gate.TryAllow(DateTimeOffset.UtcNow);
    }
}
