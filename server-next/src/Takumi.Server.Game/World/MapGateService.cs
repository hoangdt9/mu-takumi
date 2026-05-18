namespace Takumi.Server.Game.World;

/// <summary>Gate resolution (parity <c>CGate::GetGate</c> / <c>gObjMoveGate</c>).</summary>
public static class MapGateService
{
    public readonly record struct TeleportDestination(byte MapId, byte X, byte Y, byte Angle, bool MapChanged);

    public static bool TryResolveGateTeleport(
        int sourceGateIndex,
        byte playerMap,
        byte playerX,
        byte playerY,
        int playerLevel,
        int playerReset,
        int playerAccountLevel,
        byte previousMap,
        out TeleportDestination dest)
    {
        dest = default;
        if (!MapGateCatalog.TryGetGate(sourceGateIndex, out var source) || source is null)
        {
            return false;
        }

        if (!SkipProximityCheck() && !IsNearGate(source, playerMap, playerX, playerY))
        {
            return false;
        }

        if (!PassesGateRequirements(source, playerLevel, playerReset, playerAccountLevel))
        {
            return false;
        }

        var gate = source;
        var hops = 0;
        while (gate.TargetGate != 0 && hops++ < 8)
        {
            if (!MapGateCatalog.TryGetGate(gate.TargetGate, out var next) || next is null)
            {
                return false;
            }

            gate = next;
        }

        if (!TryPickTile(gate, out var x, out var y))
        {
            x = (byte)Math.Clamp((int)gate.PosX, 0, 255);
            y = (byte)Math.Clamp((int)gate.PosY, 0, 255);
        }

        var angle = (byte)Math.Clamp((int)gate.Dir, 0, 255);
        dest = new TeleportDestination(gate.MapId, x, y, angle, gate.MapId != previousMap);
        return true;
    }

    public static bool TryResolveSkillTeleport(
        byte mapId,
        byte playerX,
        byte playerY,
        byte destX,
        byte destY,
        byte angle,
        out TeleportDestination dest)
    {
        dest = default;
        if (!SkillTeleportService.TryValidateArea(mapId, playerX, playerY, destX, destY))
        {
            return false;
        }

        dest = new TeleportDestination(mapId, destX, destY, angle, false);
        return true;
    }

    /// <summary>Move-map warp: resolve destination gate without proximity (parity <c>gObjMoveGate</c> via <c>CMove::Move</c>).</summary>
    public static bool TryResolveWarpGate(int destinationGateIndex, int playerLevel, byte previousMap, out TeleportDestination dest)
    {
        dest = default;
        if (!MapGateCatalog.TryGetGate(destinationGateIndex, out var gate) || gate is null)
        {
            return false;
        }

        if (!PassesLevel(gate, playerLevel))
        {
            return false;
        }

        var hops = 0;
        while (gate.TargetGate != 0 && hops++ < 8)
        {
            if (!MapGateCatalog.TryGetGate(gate.TargetGate, out var next) || next is null)
            {
                return false;
            }

            gate = next;
        }

        if (!TryPickTile(gate, out var x, out var y))
        {
            x = (byte)Math.Clamp((int)gate.PosX, 0, 255);
            y = (byte)Math.Clamp((int)gate.PosY, 0, 255);
        }

        var angle = (byte)Math.Clamp((int)gate.Dir, 0, 255);
        dest = new TeleportDestination(gate.MapId, x, y, angle, gate.MapId != previousMap);
        return true;
    }

    static bool SkipProximityCheck() =>
        string.Equals(Environment.GetEnvironmentVariable("TAKUMI_GATE_SKIP_PROXIMITY"), "1", StringComparison.OrdinalIgnoreCase);

    static bool IsNearGate(MapGateEntry gate, byte mapId, byte x, byte y)
    {
        if (gate.MapId != mapId)
        {
            return false;
        }

        // Parity CGate::IsInGate — ±5 around TX/TY (Gate.txt columns TX/TY).
        return x >= gate.RangeTx - 5 && x <= gate.RangeTx + 5
            && y >= gate.RangeTy - 5 && y <= gate.RangeTy + 5;
    }

    static bool PassesGateRequirements(MapGateEntry gate, int level, int reset, int accountLevel)
    {
        if (gate.MinLevel >= 0 && level < gate.MinLevel)
        {
            return false;
        }

        if (gate.MaxLevel >= 0 && level > gate.MaxLevel)
        {
            return false;
        }

        if (gate.MinReset >= 0 && reset < gate.MinReset)
        {
            return false;
        }

        if (gate.MaxReset >= 0 && reset > gate.MaxReset)
        {
            return false;
        }

        if (accountLevel < gate.AccountLevel)
        {
            return false;
        }

        return true;
    }

    static bool PassesLevel(MapGateEntry gate, int level) =>
        PassesGateRequirements(gate, level, reset: 0, accountLevel: int.MaxValue);

    static bool TryPickTile(MapGateEntry gate, out byte x, out byte y)
    {
        var minX = Math.Min(gate.PosX, gate.RangeTx);
        var maxX = Math.Max(gate.PosX, gate.RangeTx);
        var minY = Math.Min(gate.PosY, gate.RangeTy);
        var maxY = Math.Max(gate.PosY, gate.RangeTy);

        for (var attempt = 0; attempt < 24; attempt++)
        {
            var px = (byte)Random.Shared.Next(minX, maxX + 1);
            var py = (byte)Random.Shared.Next(minY, maxY + 1);
            if (MapAttWalkability.CanWalk(gate.MapId, px, py))
            {
                x = px;
                y = py;
                return true;
            }
        }

        x = 0;
        y = 0;
        return false;
    }
}
