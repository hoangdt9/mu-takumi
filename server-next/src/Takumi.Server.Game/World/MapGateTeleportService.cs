namespace Takumi.Server.Game.World;

/// <summary>M8: gate proximity + destination resolution (parity <c>CGate</c> / <c>gObjMoveGate</c>).</summary>
public static class MapGateTeleportService
{
    public sealed record TeleportResult(byte MapId, byte X, byte Y, byte Dir, ushort ClientFlag, bool Accepted);

    public static bool IsPlayerInGate(MapGateEntry gate, byte mapId, byte x, byte y)
    {
        if (gate.MapId != mapId)
        {
            return false;
        }

        var cx = gate.RangeTx;
        var cy = gate.RangeTy;
        return x >= cx - 5 && x <= cx + 5 && y >= cy - 5 && y <= cy + 5;
    }

    public static bool MeetsLevelRules(MapGateEntry gate, ushort level)
    {
        if (gate.MinLevel >= 0 && level < gate.MinLevel)
        {
            return false;
        }

        if (gate.MaxLevel >= 0 && level > gate.MaxLevel)
        {
            return false;
        }

        return true;
    }

    public static TeleportResult TryTeleport(
        int gateIndex,
        byte currentMap,
        byte currentX,
        byte currentY,
        byte currentDir,
        ushort level)
    {
        MapGateCatalog.EnsureInitialized();
        if (!MapGateCatalog.TryGetGate(gateIndex, out var source) || source is null)
        {
            return Reject(currentMap, currentX, currentY, currentDir);
        }

        if (!IsPlayerInGate(source, currentMap, currentX, currentY) || !MeetsLevelRules(source, level))
        {
            return Reject(currentMap, currentX, currentY, currentDir);
        }

        var destGate = source;
        if (destGate.TargetGate != 0)
        {
            if (!MapGateCatalog.TryGetGate(destGate.TargetGate, out var target) || target is null)
            {
                return Reject(currentMap, currentX, currentY, currentDir);
            }

            destGate = target;
        }

        var (x, y) = PickSpawnTile(destGate);
        var map = destGate.MapId;
        var dir = (byte)Math.Clamp((int)destGate.Dir, 0, 7);
        var mapChanged = map != currentMap;
        var flag = (ushort)(mapChanged ? 1 : 0);
        return new TeleportResult(map, x, y, dir, flag, Accepted: true);
    }

    static TeleportResult Reject(byte map, byte x, byte y, byte dir) =>
        new(map, x, y, dir, 0, Accepted: false);

    static (byte X, byte Y) PickSpawnTile(MapGateEntry gate)
    {
        var minX = Math.Min(gate.PosX, gate.RangeTx);
        var maxX = Math.Max(gate.PosX, gate.RangeTx);
        var minY = Math.Min(gate.PosY, gate.RangeTy);
        var maxY = Math.Max(gate.PosY, gate.RangeTy);

        var spanX = maxX - minX;
        var spanY = maxY - minY;
        var x = spanX > 0 ? minX + Random.Shared.Next(spanX + 1) : minX;
        var y = spanY > 0 ? minY + Random.Shared.Next(spanY + 1) : minY;
        return ((byte)Math.Clamp(x, 0, 255), (byte)Math.Clamp(y, 0, 255));
    }
}
