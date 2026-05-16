using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>M7d: apply server→client 0x26/0x27 vitals from outbound wire to roster storage.</summary>
public static class RosterVitalsOutboundTracker
{
    public static bool TryApplyToGameEntry(GameRosterEntry entry, ReadOnlySpan<byte> outbound)
    {
        var curHp = entry.CurrentHp;
        var maxHp = entry.MaxHp;
        var curMp = entry.CurrentMp;
        var maxMp = entry.MaxMp;
        var curSd = entry.CurrentShield;
        var maxSd = entry.MaxShield;
        if (!LifeManaWire602.TryApplyVitalsFromOutbound(outbound, ref curHp, ref maxHp, ref curMp, ref maxMp, ref curSd, ref maxSd))
        {
            return false;
        }

        entry.CurrentHp = curHp;
        entry.MaxHp = maxHp;
        entry.CurrentMp = curMp;
        entry.MaxMp = maxMp;
        entry.CurrentShield = curSd;
        entry.MaxShield = maxSd;
        return true;
    }
}
