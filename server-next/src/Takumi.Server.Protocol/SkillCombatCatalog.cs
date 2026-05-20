namespace Takumi.Server.Protocol;

/// <summary>Skill combat metadata (parity client <c>SkillAttribute</c> / Season 6 wire heads).</summary>
public static class SkillCombatCatalog
{
    public const ushort EvilSpirit = 9;
    public const ushort Storm = 8;
    public const ushort FireSlash = 55;
    public const ushort PowerSlash = 56;
    public const ushort FlameStrike = 236;
    public const ushort GiganticStorm = 237;

    public static bool IsMagicGladiator(byte serverClass) =>
        CharacterSheetCalculator.ClassIndex(serverClass) == 3;

    /// <summary><c>C1/C3 0x1E</c> channel / directional magic.</summary>
    public static bool IsAreaContinueSkill(ushort skillId)
    {
        if (skillId is EvilSpirit or Storm or FireSlash or PowerSlash or FlameStrike or GiganticStorm)
        {
            return true;
        }

        if (skillId is 385 or 487 or >= 14 and <= 18 or >= 61 and <= 65)
        {
            return true;
        }

        if (skillId is >= 43 and <= 52) // BLOOD_ATT_UP .. POWER_SLASH_UP+4 (DK/MG)
        {
            return true;
        }

        if (skillId is 482 or 490 or 493) // master Power Slash / Fire Slash
        {
            return true;
        }

        if (skillId is 10 or 13 or 14) // Hell / Inferno channel
        {
            return true;
        }

        if (skillId is 378 or 483) // master Flame (continue on ground)
        {
            return true;
        }

        return skillId is 237 or 238; // Gigantic Storm / Chaotic (DL/MG wire 0x1E)
    }

    /// <summary><c>C1/C3 0x19</c> single-target magic.</summary>
    public static bool IsTargetedMagicSkill(ushort skillId)
    {
        if (IsAreaContinueSkill(skillId))
        {
            return false;
        }

        return skillId switch
        {
            1 or 2 or 3 or 4 or 5 or 7 or 12 or 17 => true,
            38 or 39 or 40 => true,
            _ => false,
        };
    }

    /// <summary><c>C1 0xDB</c> magic burst with target list.</summary>
    public static bool IsMagicBurstSkill(ushort skillId) =>
        skillId is 13 or 14 or 38 or 40 or 382;

    public static bool UsesMagicDamage(ushort skillId) =>
        IsAreaContinueSkill(skillId)
        || IsTargetedMagicSkill(skillId)
        || IsMagicBurstSkill(skillId);

    public static bool UsesPhysicalSkillDamage(ushort skillId) =>
        skillId is 19 or 20 or 21 or 22 or 23 or 41 or 42 or 44 or 47 or 57;

    public static int GetSkillBaseDamage(ushort skillId) =>
        skillId switch
        {
            EvilSpirit or >= 61 and <= 65 or >= 14 and <= 18 or 385 or 487 => 45,
            Storm => 35,
            GiganticStorm or 237 or 238 => 40,
            FlameStrike or 236 => 55,
            FireSlash or 490 or 493 => 50,
            PowerSlash or 482 or >= 48 and <= 52 => 55,
            5 or 4 => 25,
            3 => 30,
            12 or 13 or 14 => 35,
            38 or 39 or 40 => 30,
            _ => 20,
        };

    public static int GetAreaContinueRange(ushort skillId) =>
        skillId switch
        {
            EvilSpirit or Storm or >= 14 and <= 18 or >= 61 and <= 65 or 385 or 487 => 7,
            GiganticStorm or 237 or 238 => 6,
            FlameStrike or 236 => 5,
            FireSlash or 490 or 493 => 4,
            PowerSlash or 482 or >= 48 and <= 52 => 5,
            10 or 13 or 14 => 5,
            _ => 3,
        };

    public static int GetTargetedSkillRange(ushort skillId) =>
        skillId switch
        {
            3 or 4 or 5 => 6,
            12 => 7,
            _ => 8,
        };

    public static int GetSkillHitRange(ushort skillId, bool isTargetedPacket) =>
        IsAreaContinueSkill(skillId)
            ? GetAreaContinueRange(skillId)
            : isTargetedPacket || IsTargetedMagicSkill(skillId)
                ? GetTargetedSkillRange(skillId)
                : IsMagicBurstSkill(skillId)
                    ? 6
                    : 3;

    public static bool UsesWizardryDamage(ushort skillId) => UsesMagicDamage(skillId);
}
