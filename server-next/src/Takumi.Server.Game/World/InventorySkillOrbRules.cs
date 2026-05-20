using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Skill orb / scroll use (<c>ITEM_WING+7..19</c> etc.) — parity OpenMU <c>Orbs.cs</c>.</summary>
public static class InventorySkillOrbRules
{
    public readonly record struct SkillOrbLearn(ushort SkillType, byte MinLevel, byte RequiredClassIndex);

    /// <summary>Wing-group orb offset → skill (Season 6).</summary>
    static readonly Dictionary<int, SkillOrbLearn> WingOrbs = new()
    {
        [(12 * 512) + 7] = new(19, 47, 1),   // Twisting Slash (DK/MG)
        [(12 * 512) + 8] = new(8, 8, 2),     // Heal (ELF)
        [(12 * 512) + 9] = new(13, 13, 2),   // Greater Defense
        [(12 * 512) + 10] = new(14, 18, 2),  // Greater Damage
        [(12 * 512) + 11] = new(214, 3, 2),  // Summon Goblin
        [(12 * 512) + 12] = new(41, 78, 1),  // Rageful Blow (DK)
        [(12 * 512) + 13] = new(47, 20, 1),  // Impale
        [(12 * 512) + 14] = new(48, 60, 1),  // Swell Life
        [(12 * 512) + 16] = new(55, 60, 3),  // Fire Slash (MG)
        [(12 * 512) + 17] = new(52, 64, 2),  // Penetration (ELF)
        [(12 * 512) + 18] = new(51, 81, 2),  // Ice Arrow
        [(12 * 512) + 19] = new(43, 72, 1),  // Death Stab
    };

    public static bool TryGetSkillOrbLearn(int itemIndex, out SkillOrbLearn learn) =>
        WingOrbs.TryGetValue(itemIndex, out learn);

    public static bool CanCharacterLearn(byte serverClass, in SkillOrbLearn learn)
    {
        var classIndex = CharacterSheetCalculator.ClassIndex(serverClass);
        return learn.RequiredClassIndex switch
        {
            1 => classIndex is 1 or 3,
            2 => classIndex == 2,
            3 => classIndex == 3,
            _ => classIndex == learn.RequiredClassIndex,
        };
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
