namespace Takumi.Server.Game.World;

/// <summary>Resolves tile from set-base row (parity <c>CMonsterSetBase::GetPosition</c>).</summary>
public static class MonsterSpawnResolver
{
    public static bool TryResolvePosition(MonsterSetBaseEntry entry, out byte x, out byte y) =>
        TryResolvePosition(entry, requireOutsideSafeZone: false, spreadKey: 0, out x, out y);

    /// <summary>Field mob spots: prefer tiles outside ATT safe zone (town vendors / gates).</summary>
    public static bool TryResolveFieldPosition(MonsterSetBaseEntry entry, out byte x, out byte y) =>
        TryResolveFieldPosition(entry, spreadKey: 0, out x, out y);

    public static bool TryResolveFieldPosition(MonsterSetBaseEntry entry, int spreadKey, out byte x, out byte y) =>
        TryResolvePosition(entry, requireOutsideSafeZone: true, spreadKey, out x, out y);

    static bool TryResolvePosition(
        MonsterSetBaseEntry entry,
        bool requireOutsideSafeZone,
        int spreadKey,
        out byte x,
        out byte y)
    {
        x = 0;
        y = 0;
        var mapId = entry.Map;
        if (entry.SpawnType is 0 or 4)
        {
            x = (byte)entry.X;
            y = (byte)entry.Y;
            if (requireOutsideSafeZone && MapAttWalkability.IsSafeZone(mapId, x, y))
            {
                return MapAttWalkability.TryFindNearestNonSafeWalkable(mapId, x, y, out x, out y);
            }

            return true;
        }

        if (entry.SpawnType is 1 or 3)
        {
            return TryBoxPosition(
                mapId,
                entry.X,
                entry.Y,
                entry.Tx,
                entry.Ty,
                requireOutsideSafeZone,
                spreadKey,
                out x,
                out y);
        }

        if (entry.SpawnType == 2)
        {
            return TryBoxPosition(
                mapId,
                entry.X - 3,
                entry.Y - 3,
                entry.X + 3,
                entry.Y + 3,
                requireOutsideSafeZone,
                spreadKey,
                out x,
                out y);
        }

        return false;
    }

    static bool TryBoxPosition(
        byte mapId,
        int x,
        int y,
        int tx,
        int ty,
        bool requireOutsideSafeZone,
        int spreadKey,
        out byte ox,
        out byte oy)
    {
        ox = 0;
        oy = 0;
        var minX = Math.Min(x, tx);
        var maxX = Math.Max(x, tx);
        var minY = Math.Min(y, ty);
        var maxY = Math.Max(y, ty);
        var rng = CreateSpreadRng(mapId, minX, minY, maxX, maxY, spreadKey);

        for (var n = 0; n < 100; n++)
        {
            var subx = Math.Max(1, maxX - minX);
            var suby = Math.Max(1, maxY - minY);
            var rx = minX + rng.Next(subx + 1);
            var ry = minY + rng.Next(suby + 1);
            if (rx is < 0 or > 255 || ry is < 0 or > 255)
            {
                continue;
            }

            ox = (byte)rx;
            oy = (byte)ry;
            if (!IsValidFieldTile(mapId, ox, oy, requireOutsideSafeZone))
            {
                continue;
            }

            return true;
        }

        if (!requireOutsideSafeZone || !MapAttWalkability.IsAttLoaded(mapId))
        {
            return false;
        }

        var centerX = (byte)Math.Clamp((minX + maxX) / 2, 0, 255);
        var centerY = (byte)Math.Clamp((minY + maxY) / 2, 0, 255);
        if (MapAttWalkability.TryFindNearestNonSafeWalkable(mapId, centerX, centerY, out ox, out oy)
            && IsValidFieldTile(mapId, ox, oy, requireOutsideSafeZone: true))
        {
            return true;
        }

        return MapAttWalkability.TryFindNearestNonSafeWalkable(mapId, centerX, centerY, out ox, out oy);
    }

    static bool IsValidFieldTile(byte mapId, byte x, byte y, bool requireOutsideSafeZone)
    {
        if (MapAttWalkability.IsAttLoaded(mapId) && !MapAttWalkability.CanWalk(mapId, x, y))
        {
            return false;
        }

        return !requireOutsideSafeZone || !MapAttWalkability.IsSafeZone(mapId, x, y);
    }

    static Random CreateSpreadRng(byte mapId, int minX, int minY, int maxX, int maxY, int spreadKey)
    {
        if (spreadKey == 0)
        {
            return Random.Shared;
        }

        var seed = HashCode.Combine(mapId, minX, minY, maxX, maxY, spreadKey);
        return new Random(seed);
    }
}
