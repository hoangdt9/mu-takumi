using System.Collections.Concurrent;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Tracks active combat buffs per game session for HUD <c>F3 E1</c> refresh.</summary>
public static class PlayerCombatEffectSession
{
    static readonly ConcurrentDictionary<Guid, CombatEffectState602> Sessions = new();

    public static CombatEffectState602 GetOrEmpty(Guid? presenceSessionId) =>
        presenceSessionId is { } sid && Sessions.TryGetValue(sid, out var state)
            ? state
            : CombatEffectState602.Empty;

    public static bool TryRegisterSelfBuff(
        Guid? presenceSessionId,
        ushort skillId,
        byte serverClass,
        CharacterSheetStats sheet,
        CharacterCombatAccumulator acc)
    {
        if (presenceSessionId is not { } sid)
        {
            return false;
        }

        var state = Sessions.GetOrAdd(sid, _ => new CombatEffectState602());
        state.AddPhysiDamage = 0;
        state.AddMagicDamage = 0;
        state.AddCurseDamage = 0;
        state.AddDefense = 0;
        state.AddSwordPowerDamageRate = 0;
        state.AddSwordPowerDefenseRate = 0;
        state.MulPhysiDamage = 0;
        state.MulMagicDamage = 0;
        state.MulCurseDamage = 0;

        return SkillBuffPreview602.TryApplyBuff(skillId, serverClass, sheet, acc, state);
    }

    public static void Clear(Guid presenceSessionId) => Sessions.TryRemove(presenceSessionId, out _);
}
