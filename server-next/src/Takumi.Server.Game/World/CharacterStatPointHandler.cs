using System.Buffers.Binary;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary><c>CGLevelUpPointRecv</c> — <c>C1 F3 06</c> stat allocation (legacy 5-byte or bulk 7-byte with count).</summary>
public static class CharacterStatPointHandler
{
    public const int LegacyRequestLength = 5;
    public const int BulkRequestLength = 7;

    public static bool TryFindNextAddPointRequest(
        ReadOnlySpan<byte> packet,
        ref int searchFrom,
        out byte statType,
        out int count)
    {
        statType = 0;
        count = 0;
        for (var i = searchFrom; i + LegacyRequestLength <= packet.Length; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 2] != 0xF3 || packet[i + 3] != 0x06)
            {
                continue;
            }

            var packetSize = packet[i + 1];
            if (packetSize < LegacyRequestLength || i + packetSize > packet.Length)
            {
                continue;
            }

            statType = packet[i + 4];
            count = 1;
            if (packetSize >= BulkRequestLength)
            {
                count = BinaryPrimitives.ReadUInt16LittleEndian(packet.Slice(i + 5, 2));
                if (count <= 0)
                {
                    count = 1;
                }
            }

            searchFrom = i + packetSize;
            return true;
        }

        searchFrom = packet.Length;
        return false;
    }

    public static async Task<bool> TryHandleAsync(
        GameRosterEntry player,
        string? accountId,
        byte[] packet,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        CancellationToken ct)
    {
        var span = packet.AsSpan();
        var searchFrom = 0;
        var sheet = player.ResolveSheet();
        var handledAny = false;
        byte lastStatType = 0;
        var failed = false;

        while (TryFindNextAddPointRequest(span, ref searchFrom, out var statType, out var count))
        {
            handledAny = true;
            lastStatType = statType;
            var applied = CharacterSheetCalculator.TryAddStatPoints(ref sheet, statType, count, out _);
            if (applied <= 0)
            {
                failed = true;
                break;
            }
        }

        if (!handledAny)
        {
            return false;
        }

        if (failed)
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
        RosterProgressMirror.ScheduleFromEntry(accountId, player);

        var maxLifeOrMana = CharacterSheetCalculator.MaxAfterStatAdd(player.ServerClass, player.Level, sheet, lastStatType);
        var pkt = LevelUpPointWire602.BuildSuccess(lastStatType, sheet, vitals, maxLifeOrMana);
        await writeAsync(pkt, ct).ConfigureAwait(false);
        Console.WriteLine(
            "[m7] stat point bulk type={0} str={1} vit={2} ene={3} pointsLeft={4}",
            lastStatType,
            sheet.Strength,
            sheet.Vitality,
            sheet.Energy,
            sheet.LevelUpPoint);
        return true;
    }
}
