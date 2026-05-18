using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Magic teleport (<c>CGTeleportRecv</c> <c>gate==0</c>, parity <c>SKILL_TELEPORT</c> + <c>gObjCheckTeleportArea</c>).</summary>
public static class SkillTeleportService
{
    public const ushort SkillIndex = 6;

    public const int RangeTiles = 8;

    public static bool SkipChecks() =>
        string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_SKILL_TELEPORT_SKIP"),
            "1",
            StringComparison.OrdinalIgnoreCase);

    public static bool PlayerHasTeleportSkill(byte serverClass) =>
        SkipChecks() || CharacterSheetCalculator.ClassIndex(serverClass) == 3;

    /// <summary>Parity <c>gObjCheckTeleportArea</c> (±8, walkable dest, no safe zone).</summary>
    public static bool TryValidateArea(byte mapId, byte playerX, byte playerY, byte destX, byte destY)
    {
        if (Math.Abs(destX - playerX) > RangeTiles || Math.Abs(destY - playerY) > RangeTiles)
        {
            return false;
        }

        if (!MapAttWalkability.CanWalk(mapId, destX, destY))
        {
            return false;
        }

        if (MapAttWalkability.IsSafeZone(mapId, playerX, playerY)
            || MapAttWalkability.IsSafeZone(mapId, destX, destY))
        {
            return false;
        }

        return true;
    }

    public static bool TryConsumeResources(GameRosterEntry player, out int manaSpent)
    {
        manaSpent = 0;
        if (SkipChecks())
        {
            return true;
        }

        if (!PlayerHasTeleportSkill(player.ServerClass))
        {
            return false;
        }

        var manaCost = ReadIntEnv("TAKUMI_SKILL_TELEPORT_MANA", 30);
        var bpCost = ReadIntEnv("TAKUMI_SKILL_TELEPORT_BP", 0);
        if (player.CurrentMp < manaCost || player.CurrentBp < bpCost)
        {
            return false;
        }

        manaSpent = manaCost;
        player.CurrentMp -= manaCost;
        if (bpCost > 0)
        {
            player.CurrentBp -= bpCost;
        }

        return true;
    }

    static int ReadIntEnv(string name, int defaultValue)
    {
        var raw = Environment.GetEnvironmentVariable(name);
        return int.TryParse(raw, out var v) && v >= 0 ? v : defaultValue;
    }
}
