namespace Takumi.Server.Protocol;

/// <summary>Tile-space facing checks for directional <c>0x1E</c> channel skills (MG slashes, Flame Strike, …).</summary>
public static class SkillCombatDirection
{
    /// <summary>Wire angle byte 0–255 maps to a full turn (client <c>Angle/360*256</c>).</summary>
    public static double WireAngleToDegrees(byte wireAngle256) =>
        wireAngle256 * 360.0 / 256.0;

    /// <summary>
    /// Facing for hit volumes — OpenMU <c>FrustumBasedTargetFilter</c> adds 180° to wire rotation.
    /// </summary>
    public static double WireAngleToFacingDegrees(byte wireAngle256) =>
        (WireAngleToDegrees(wireAngle256) + 180.0) % 360.0;

    /// <summary>
    /// True when <paramref name="targetX"/>/<paramref name="targetY"/> lies within Manhattan
    /// <paramref name="maxRange"/> tiles of origin and inside a forward arc (~140° default).
    /// </summary>
    public static bool IsInForwardArc(
        byte originX,
        byte originY,
        byte facingWire256,
        byte targetX,
        byte targetY,
        int maxRange,
        double halfArcDegrees = 70.0)
    {
        var dx = targetX - originX;
        var dy = targetY - originY;
        var dist = Math.Abs(dx) + Math.Abs(dy);
        if (dist == 0 || dist > maxRange)
        {
            return false;
        }

        var facingRad = WireAngleToFacingDegrees(facingWire256) * Math.PI / 180.0;
        var targetRad = Math.Atan2(dy, dx);
        var delta = NormalizeRadians(targetRad - facingRad);
        var halfArcRad = halfArcDegrees * Math.PI / 180.0;
        return Math.Abs(delta) <= halfArcRad;
    }

    /// <summary>
    /// Narrow forward strip for Twister (parity OpenMU <c>FrustumBasedTargetFilter</c>, width 1.5).
    /// Targets must be in front of the caster within <paramref name="maxForwardTiles"/> and
    /// within <paramref name="halfWidthTiles"/> perpendicular distance from the center line.
    /// </summary>
    public static bool IsInForwardCorridor(
        byte originX,
        byte originY,
        byte facingWire256,
        byte targetX,
        byte targetY,
        int maxForwardTiles,
        float halfWidthTiles = 1.5f)
    {
        var dx = targetX - originX;
        var dy = targetY - originY;
        if (dx == 0 && dy == 0)
        {
            return false;
        }

        var facingRad = WireAngleToFacingDegrees(facingWire256) * Math.PI / 180.0;
        var forwardX = Math.Cos(facingRad);
        var forwardY = Math.Sin(facingRad);
        var forward = (dx * forwardX) + (dy * forwardY);
        if (forward <= 0.25 || forward > maxForwardTiles + 0.5)
        {
            return false;
        }

        var perp = Math.Abs((dx * forwardY) - (dy * forwardX));
        return perp <= halfWidthTiles;
    }

    static double NormalizeRadians(double radians)
    {
        while (radians > Math.PI)
        {
            radians -= 2.0 * Math.PI;
        }

        while (radians < -Math.PI)
        {
            radians += 2.0 * Math.PI;
        }

        return radians;
    }
}
