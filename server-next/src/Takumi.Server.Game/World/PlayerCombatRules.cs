using System.Globalization;

namespace Takumi.Server.Game.World;

/// <summary>PvP gates: enable flag, melee range, safe zone (parity <c>gObjCheckAttackArea</c> stub).</summary>
public static class PlayerCombatRules
{
    public static bool IsPvPEnabled() =>
        !string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_COMBAT_PVP_ENABLED")?.Trim(),
            "0",
            StringComparison.OrdinalIgnoreCase);

    public static int MeleeRange() =>
        ParseIntEnv("TAKUMI_COMBAT_PVP_MELEE_RANGE", ParseIntEnv("TAKUMI_COMBAT_MELEE_RANGE", 3, 1, 15), 1, 15);

    public static bool IsSafeZoneBlocked(byte mapId, byte x, byte y) =>
        MapAttWalkability.IsSafeZone(mapId, x, y);

    public static bool IsInMeleeRange(byte ax, byte ay, byte tx, byte ty, int? range = null)
    {
        var r = range ?? MeleeRange();
        return Math.Abs(tx - ax) + Math.Abs(ty - ay) <= r;
    }

    public static bool CanAttackPlayer(
        byte mapId,
        byte attackerX,
        byte attackerY,
        byte victimX,
        byte victimY)
    {
        if (!IsPvPEnabled())
        {
            return false;
        }

        if (IsSafeZoneBlocked(mapId, attackerX, attackerY) || IsSafeZoneBlocked(mapId, victimX, victimY))
        {
            return false;
        }

        return IsInMeleeRange(attackerX, attackerY, victimX, victimY);
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
