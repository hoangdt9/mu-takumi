namespace Takumi.Server.Protocol;

/// <summary>Skill combat metadata (parity client <c>SkillAttribute</c> / Season 6 wire heads).</summary>
public static class SkillCombatCatalog
{
    public const ushort EvilSpirit = 9;
    /// <summary>Twister (<c>AT_SKILL_STORM</c>).</summary>
    public const ushort Twister = 8;
    public const ushort Storm = Twister;
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

    /// <summary>Wire uses magic handlers; stat roll may still be physical (MG slashes).</summary>
    public static bool UsesMagicDamage(ushort skillId) =>
        IsAreaContinueSkill(skillId)
        || IsTargetedMagicSkill(skillId)
        || IsMagicBurstSkill(skillId);

    /// <summary>
    /// Webzen <c>Attack.cpp</c>: MG/DW magic except melee slash list uses <c>GetAttackDamageWizard</c>.
    /// MG Fire Slash / Power Slash / Flame Strike use <c>GetAttackDamage</c> (physical + skill value).
    /// </summary>
    public static bool UsesPhysicalStatRoll(ushort skillId)
    {
        if (skillId is 19 or 20 or 21 or 22 or 23 or 41 or 42 or 44 or 47 or 57)
        {
            return true;
        }

        return skillId is FireSlash or PowerSlash or FlameStrike
            or 482 or 490 or 493
            or >= 48 and <= 52;
    }

    public static bool UsesWizardryStatRoll(ushort skillId) =>
        UsesMagicDamage(skillId) && !UsesPhysicalStatRoll(skillId);

    public static bool UsesPhysicalSkillDamage(ushort skillId) => UsesPhysicalStatRoll(skillId);

    public static int GetSkillBaseDamage(ushort skillId) =>
        skillId switch
        {
            EvilSpirit or >= 61 and <= 65 or >= 14 and <= 18 or 385 or 487 => 45,
            Storm => 35,
            GiganticStorm or 237 or 238 => 110,
            FlameStrike or 236 => 140,
            FireSlash or 490 or 493 => 80,
            PowerSlash or 482 or >= 48 and <= 52 => 0,
            5 or 4 => 25,
            3 => 30,
            12 or 13 or 14 => 35,
            38 or 39 or 40 => 30,
            _ => 20,
        };

    /// <summary>
    /// Directional channel skills (MG slashes / flame strike): forward arc (~140°).
    /// Gigantic Storm (237) is omnidirectional: client spawns five bolts every 72° around the caster.
    /// </summary>
    public static bool IsDirectionalContinueSkill(ushort skillId) =>
        skillId is FireSlash or PowerSlash or FlameStrike
            or 490 or 493 or 482 or >= 48 and <= 52
            or 10 or 13 or 378 or 483;

    /// <summary>
    /// Narrow forward corridor (OpenMU frustum: start/end width 1.5, distance 4, skill range 6).
    /// Twister only travels along the tornado path — not a wide square.
    /// </summary>
    public static bool IsForwardCorridorContinueSkill(ushort skillId) =>
        skillId is Twister;

    /// <summary>Half-width of the forward corridor in tile units (OpenMU <c>1.5f</c>).</summary>
    public const float ForwardCorridorHalfWidth = 1.5f;

    /// <summary>Forward depth for corridor skills (Twister frustum distance 4; not full skill range 6).</summary>
    public static int GetForwardCorridorMaxTiles(ushort skillId) =>
        skillId is Twister ? 4 : GetAreaContinueRange(skillId);

    /// <summary>
    /// Max Chebyshev hit distance from caster tile for omnidirectional skills, Manhattan arc depth for directional.
    /// Evil Spirit uses OpenMU Season 6 distance 7; other values follow <c>Skill.txt</c> Radio.
    /// </summary>
    public static int GetAreaContinueRange(ushort skillId) =>
        skillId switch
        {
            // OpenMU S6 CreateSkill(EvilSpirit, …, distance: 7); Skill.txt Radio=6 is client UI only.
            EvilSpirit or >= 61 and <= 65 or 385 or 487 => 7,
            // Twister / Inferno channel: Radio=6 in Skill.txt
            Storm or >= 14 and <= 18 => 6,
            // Skill.txt Radio=6; OpenMU GiganticStorm distance=6 (not 3 like Flame Strike).
            GiganticStorm or 237 or 238 => 6,
            FlameStrike or 236 => 2,
            FireSlash or 490 or 493 => 2,
            PowerSlash or 482 or >= 48 and <= 52 => 2,
            10 or 13 => 5,
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

    /// <summary>
    /// OpenMU <c>Stats.SkillFinalMultiplier</c> after defense — skill hits only (melee = 100).
    /// Ensures skill &gt; normal attack when base+def are similar.
    /// </summary>
    public static int GetSkillFinalMultiplierPercent(ushort skillId)
    {
        if (skillId == 0)
        {
            return 100;
        }

        return skillId switch
        {
            EvilSpirit or 385 or 487 => 150,
            Storm => 130,
            GiganticStorm or 237 => 160,
            238 => 150,
            FlameStrike or 236 => 170,
            FireSlash or 490 or 493 => 155,
            PowerSlash or 482 or >= 48 and <= 52 => 150,
            >= 61 and <= 65 => 140,
            10 or 13 or 14 => 140,
            3 or 4 or 5 or 12 => 130,
            38 or 39 or 40 => 140,
            19 or 20 or 21 or 22 or 23 or 41 or 42 or 44 or 47 or 57 => 130,
            _ => 145,
        };
    }
}
