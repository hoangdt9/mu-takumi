using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>M7d helpers shared by <c>GamePortMinimalSession</c> and documented for <c>LegacyLoginHost</c>.</summary>
public static class RosterVitalsLifecycle
{
    public static bool TrySeedGameEntryFromJoin(GameRosterEntry entry, ReadOnlySpan<byte> joinPkt)
    {
        if (!JoinMapVitalsSeed.TryApplyFromJoinPacketIfUnset(entry.MaxHp > 0, joinPkt, out var v))
        {
            return false;
        }

        entry.CurrentHp = v.CurrentHp;
        entry.MaxHp = v.MaxHp;
        entry.CurrentMp = v.CurrentMp;
        entry.MaxMp = v.MaxMp;
        entry.Zen = v.Zen;
        return true;
    }

    public static void ApplyVitals(CharacterRosterVitals v, ref int currentHp, ref int maxHp, ref int currentMp, ref int maxMp, ref long zen)
    {
        currentHp = v.CurrentHp;
        maxHp = v.MaxHp;
        currentMp = v.CurrentMp;
        maxMp = v.MaxMp;
        zen = v.Zen;
    }
}
