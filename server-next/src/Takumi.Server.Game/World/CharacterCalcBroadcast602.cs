using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Sends <c>C1 F3 E1</c> after stat or equipment changes.</summary>
public static class CharacterCalcBroadcast602
{
    public static async Task SendAsync(
        GameRosterEntry player,
        Guid? presenceSessionId,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        CancellationToken ct)
    {
        var pkt = BuildCalcPacket(player, presenceSessionId, ResolveWearSlots(presenceSessionId));
        await writeAsync(pkt, ct).ConfigureAwait(false);
    }

    public static IReadOnlyDictionary<byte, byte[]>? ResolveWearSlots(Guid? presenceSessionId)
    {
        if (presenceSessionId is not { } sid
            || !PlayerShopSession.TryGetSessionSlots(sid, out var slots))
        {
            return null;
        }

        return slots
            .Where(kv => ItemWire602.IsWearSlot(kv.Key) && !ItemWire602.IsEmpty(kv.Value))
            .ToDictionary(kv => kv.Key, kv => kv.Value);
    }

    public static byte[] BuildCalcPacket(
        GameRosterEntry player,
        Guid? presenceSessionId,
        IReadOnlyDictionary<byte, byte[]>? wearSlots)
    {
        var roster = player.ToWireWithSheet();
        var lv = Math.Max((ushort)1, roster.Level);
        var sheet = CharacterSheetCalculator.ResolveSheet(roster.ServerClass, lv, roster.Sheet);
        var vitals = CharacterSheetCalculator.ComputeMaxVitals(roster.ServerClass, lv, sheet);
        var merged = CharacterSheetCalculator.MergeVitalsForJoin(roster.Vitals, vitals);
        vitals = vitals with
        {
            Life = (ushort)Math.Clamp(merged.CurrentHp > 0 ? merged.CurrentHp : merged.MaxHp, 0, ushort.MaxValue),
            Mana = (ushort)Math.Clamp(merged.CurrentMp > 0 ? merged.CurrentMp : merged.MaxMp, 0, ushort.MaxValue),
            Shield = (ushort)Math.Clamp(merged.CurrentShield, 0, ushort.MaxValue),
        };

        var effects = PlayerCombatEffectSession.GetOrEmpty(presenceSessionId);
        var calc = CharacterCombatCalculator602.Compute(roster.ServerClass, lv, sheet, wearSlots, effects);
        return NewCharacterCalcWire602.Build(roster, vitals, calc);
    }
}
