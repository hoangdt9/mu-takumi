using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>
/// Skill orb / scroll / crystal use (<c>ITEM_WING+7..48</c>, group 12) — parity OpenMU Season6 <c>Orbs.cs</c>.
/// <see cref="AllowedClassMask"/> uses <see cref="CharacterSheetCalculator.ClassIndex"/> bits: 0=DW,1=DK,2=FE,3=MG,4=DL,5=SU,6=RF.
/// </summary>
public static class InventorySkillOrbRules
{
    /// <param name="MinEnergy">Total energy required; 0 = do not validate (client already gated).</param>
    public readonly record struct SkillOrbLearn(ushort SkillType, byte MinLevel, uint AllowedClassMask, ushort MinEnergy = 0);

    const uint Dk = 1u << 1;
    const uint Fe = 1u << 2;
    const uint Mg = 1u << 3;
    const uint Dl = 1u << 4;
    const uint Su = 1u << 5;
    const uint Rf = 1u << 6;

    /// <summary>Item index <c>group*512 + number</c> (orbs group = 12).</summary>
    static readonly Dictionary<int, SkillOrbLearn> WingOrbs = new()
    {
        // --- Core orbs (OpenMU Orbs.cs lines 36–49) ---
        [(12 * 512) + 7] = new(19, 80, Dk | Mg), // Twisting Slash
        [(12 * 512) + 8] = new(26, 8, Fe), // Heal (client AT_SKILL_HEALING)
        [(12 * 512) + 9] = new(27, 13, Fe), // Greater Defense
        [(12 * 512) + 10] = new(28, 18, Fe), // Greater Damage
        [(12 * 512) + 11] = new(30, 1, Fe), // Summon Goblin (AT_SKILL_SUMMON); OpenMU level req 0 — use 1
        [(12 * 512) + 12] = new(41, 170, Dk), // Rageful Blow
        [(12 * 512) + 13] = new(47, 28, Dk | Mg), // Impale
        [(12 * 512) + 14] = new(48, 120, Dk), // Swell Life
        [(12 * 512) + 16] = new(55, 60, Mg), // Fire Slash
        [(12 * 512) + 17] = new(52, 64, Fe), // Penetration
        [(12 * 512) + 18] = new(51, 81, Fe), // Ice Arrow
        [(12 * 512) + 19] = new(43, 72, Dk), // Death Stab
        // --- Crystals (same item group) ---
        [(12 * 512) + 44] = new(232, 220, Dk), // Strike of Destruction
        [(12 * 512) + 45] = new(235, 220, Fe), // Multi-Shot
        [(12 * 512) + 46] = new(234, 220, Fe), // Recovery
        [(12 * 512) + 47] = new(236, 220, Mg), // Flame Strike
        [(12 * 512) + 48] = new(238, 220, Dl), // Chaotic Diseier
        // --- Scrolls (DL / SU / RF) ---
        [(12 * 512) + 21] = new(61, 74, Dl), // Fire Burst (OpenMU SkillNumber)
        [(12 * 512) + 22] = new(63, 98, Su), // Summon
        [(12 * 512) + 23] = new(64, 82, Su), // Increase Critical Damage
        [(12 * 512) + 24] = new(65, 92, Rf), // Electric Spike
        [(12 * 512) + 35] = new(78, 102, Dl), // Fire Scream
    };

    public static bool TryGetSkillOrbLearn(int itemIndex, out SkillOrbLearn learn) =>
        WingOrbs.TryGetValue(itemIndex, out learn);

    public static bool CanCharacterLearn(byte serverClass, in SkillOrbLearn learn)
    {
        var classIndex = CharacterSheetCalculator.ClassIndex(serverClass);
        if (classIndex < 0 || classIndex > 6)
        {
            return false;
        }

        return (learn.AllowedClassMask & (1u << classIndex)) != 0;
    }

    public static byte PickSkillSlot(ushort skillType, IReadOnlyList<CharacterSkillRow> existing)
    {
        foreach (var row in existing)
        {
            if (row.Type == skillType)
            {
                return row.Slot;
            }
        }

        if (skillType <= byte.MaxValue)
        {
            var preferred = (byte)skillType;
            if (existing.All(r => r.Slot != preferred))
            {
                return preferred;
            }
        }

        for (byte slot = 0; slot < byte.MaxValue; slot++)
        {
            if (existing.All(r => r.Slot != slot))
            {
                return slot;
            }
        }

        return 0;
    }
}
