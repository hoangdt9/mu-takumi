namespace Takumi.Server.Protocol;

/// <summary>Tile distance checks for area skills (parity OpenMU <c>IsInRange</c> / client <c>Skill.txt</c> Radio).</summary>
public static class SkillCombatRange
{
    /// <summary>Chebyshev (square): max(|dx|,|dy|) ≤ range — matches OpenMU area queries.</summary>
    public static bool IsWithinChebyshev(byte centerX, byte centerY, byte targetX, byte targetY, int range)
    {
        var dx = Math.Abs(targetX - centerX);
        var dy = Math.Abs(targetY - centerY);
        return Math.Max(dx, dy) <= range;
    }

    /// <summary>Manhattan (diamond): |dx|+|dy| ≤ range.</summary>
    public static bool IsWithinManhattan(byte centerX, byte centerY, byte targetX, byte targetY, int range)
    {
        return Math.Abs(targetX - centerX) + Math.Abs(targetY - centerY) <= range;
    }

    /// <summary>
    /// Omnidirectional <c>0x1E</c> skills (Evil Spirit, Storm, …) always radiate from the caster tile.
    /// Directional slashes also use the player tile as the arc origin.
    /// </summary>
    public static (byte X, byte Y) GetAreaContinueCenter(
        byte playerX,
        byte playerY,
        byte skillX,
        byte skillY,
        bool directional)
    {
        _ = skillX;
        _ = skillY;
        _ = directional;
        return (playerX, playerY);
    }

    /// <summary>Whether a mob tile is inside the skill's hit volume.</summary>
    public static bool IsMobInSkillVolume(
        ushort skillId,
        byte centerX,
        byte centerY,
        byte facingWire256,
        byte mobX,
        byte mobY,
        int range)
    {
        if (SkillCombatCatalog.IsDirectionalContinueSkill(skillId))
        {
            return SkillCombatDirection.IsInForwardArc(
                centerX,
                centerY,
                facingWire256,
                mobX,
                mobY,
                range);
        }

        if (SkillCombatCatalog.IsForwardCorridorContinueSkill(skillId))
        {
            return SkillCombatDirection.IsInForwardCorridor(
                centerX,
                centerY,
                facingWire256,
                mobX,
                mobY,
                maxForwardTiles: range,
                SkillCombatCatalog.ForwardCorridorHalfWidth);
        }

        return IsWithinChebyshev(centerX, centerY, mobX, mobY, range);
    }
}
