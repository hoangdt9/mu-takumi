using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary><c>CGLevelUpPointRecv</c> — <c>C1 F3 06</c> stat allocation.</summary>
public static class CharacterStatPointHandler
{
    public static bool TryFindAddPointRequest(ReadOnlySpan<byte> packet, out byte statType)
    {
        statType = 0;
        for (var i = 0; i + 5 <= packet.Length; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 2] != 0xF3 || packet[i + 3] != 0x06)
            {
                continue;
            }

            statType = packet[i + 4];
            return true;
        }

        return false;
    }

    public static async Task<bool> TryHandleAsync(
        GameRosterEntry player,
        byte[] packet,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        CancellationToken ct)
    {
        if (!TryFindAddPointRequest(packet.AsSpan(), out var statType))
        {
            return false;
        }

        var sheet = player.ResolveSheet();
        if (!CharacterSheetCalculator.TryAddStatPoint(ref sheet, statType, out _))
        {
            await writeAsync(LevelUpPointWire602.BuildFail(), ct).ConfigureAwait(false);
            return true;
        }

        player.ApplySheet(sheet);
        var vitals = CharacterSheetCalculator.ComputeMaxVitals(player.ServerClass, player.Level, sheet);
        player.MaxHp = vitals.LifeMax;
        player.CurrentHp = vitals.Life;
        player.MaxMp = vitals.ManaMax;
        player.CurrentMp = vitals.Mana;
        player.MaxShield = vitals.ShieldMax;
        player.CurrentShield = vitals.Shield;
        player.CurrentBp = vitals.SkillMana;
        player.MaxBp = vitals.SkillManaMax;
        onRosterDirty?.Invoke();

        var pkt = LevelUpPointWire602.BuildSuccess(statType, sheet, vitals);
        await writeAsync(pkt, ct).ConfigureAwait(false);
        Console.WriteLine(
            "[m7] stat point type={0} str={1} vit={2} ene={3} pointsLeft={4}",
            statType,
            sheet.Strength,
            sheet.Vitality,
            sheet.Energy,
            sheet.LevelUpPoint);
        return true;
    }
}
