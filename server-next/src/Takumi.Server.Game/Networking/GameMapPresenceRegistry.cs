using System.Collections.Concurrent;
using MUnique.OpenMU.Network;
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
}

public static class GameMapPresenceRegistry
{
    static readonly ConcurrentDictionary<Guid, MapPresenceSession> Sessions = new();
    static int _nextPlayerKey = 1000;

    public static bool IsEnabled =>
        !string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_MAP_PRESENCE_ENABLED")?.Trim(),
            "0",
            StringComparison.OrdinalIgnoreCase);

    public static MapPresenceSession? Register(
        Guid sessionId,
        Connection connection,
        (byte K1, byte K2)? protect,
        byte mapId,
        byte x,
        byte y,
        byte angle)
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
        };
        Sessions[sessionId] = session;
        return session;
    }

    public static void Unregister(Guid sessionId) => Sessions.TryRemove(sessionId, out _);

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
            var otherPos = PlayerPositionWire602.Build(other.ObjectKey, other.X, other.Y);
            await GamePortOutboundWire.WriteAsync(self.Connection, self.Protect, otherPos, ct).ConfigureAwait(false);
            var selfPos = PlayerPositionWire602.Build(self.ObjectKey, self.X, self.Y);
            await GamePortOutboundWire.WriteAsync(other.Connection, other.Protect, selfPos, ct).ConfigureAwait(false);
        }

        Console.WriteLine(
            "[{0}] [m10] map presence join map={1} key={2} peers={3}",
            remote,
            self.MapId,
            self.ObjectKey,
            peerCount);
    }

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
}
