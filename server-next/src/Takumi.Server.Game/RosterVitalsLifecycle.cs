using System.Globalization;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>M7d helpers shared by <c>GamePortMinimalSession</c> and <c>LegacyLoginHostRunner</c>.</summary>
public static class RosterVitalsLifecycle
{
    public static bool TryApplyOutbound(
        ReadOnlySpan<byte> outbound,
        ref int currentHp,
        ref int maxHp,
        ref int currentMp,
        ref int maxMp) =>
        LifeManaWire602.TryApplyVitalsFromOutbound(outbound, ref currentHp, ref maxHp, ref currentMp, ref maxMp);

    public static bool IsSendLifeManaAfterJoinEnabled()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_SEND_LIFE_MANA_AFTER_JOIN")?.Trim();
        if (string.IsNullOrEmpty(raw))
        {
            return true;
        }

        return !string.Equals(raw, "0", StringComparison.OrdinalIgnoreCase)
               && !string.Equals(raw, "false", StringComparison.OrdinalIgnoreCase)
               && !string.Equals(raw, "no", StringComparison.OrdinalIgnoreCase);
    }

    public static async Task TrySendLifeManaSyncAsync(
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        int currentHp,
        int maxHp,
        int currentMp,
        int maxMp,
        CancellationToken ct)
    {
        if (!IsSendLifeManaAfterJoinEnabled() || maxHp <= 0)
        {
            return;
        }

        var hp = (ushort)Math.Clamp(currentHp > 0 ? currentHp : maxHp, 0, ushort.MaxValue);
        var hpMax = (ushort)Math.Clamp(maxHp, 0, ushort.MaxValue);
        var mp = (ushort)Math.Clamp(currentMp > 0 ? currentMp : maxMp, 0, ushort.MaxValue);
        var mpMax = (ushort)Math.Clamp(maxMp, 0, ushort.MaxValue);

        await writeAsync(LifeManaWire602.BuildLife(LifeManaWire602.TypeCurrent, hp), ct).ConfigureAwait(false);
        await writeAsync(LifeManaWire602.BuildLife(LifeManaWire602.TypeMax, hpMax), ct).ConfigureAwait(false);
        await writeAsync(LifeManaWire602.BuildMana(LifeManaWire602.TypeCurrent, mp), ct).ConfigureAwait(false);
        await writeAsync(LifeManaWire602.BuildMana(LifeManaWire602.TypeMax, mpMax), ct).ConfigureAwait(false);
    }

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
