using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>M7d: apply player HP/MP/SD changes and emit <c>0x26</c>/<c>0x27</c> (legacy <c>GCLifeSend</c>).</summary>
public static class RosterVitalsCombat
{
    public static async Task<bool> ApplyPlayerDamageAsync(
        GameRosterEntry player,
        int damage,
        int playerObjectKey,
        int killerObjectKey,
        string? accountId,
        string? characterName,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        CancellationToken ct)
    {
        if (damage <= 0 || player.MaxHp <= 0)
        {
            return false;
        }

        var maxHp = Math.Max(1, player.MaxHp);
        var maxSd = Math.Max(0, player.MaxShield);
        var sd = Math.Max(0, player.CurrentShield);
        if (maxSd > 0 && sd <= 0)
        {
            sd = maxSd;
        }

        var remaining = damage;
        if (maxSd > 0)
        {
            var takeSd = Math.Min(sd, remaining);
            sd -= takeSd;
            remaining -= takeSd;
        }

        player.MaxShield = maxSd;
        player.CurrentShield = sd;

        var beforeHp = player.CurrentHp > 0 ? player.CurrentHp : maxHp;
        player.CurrentHp = Math.Max(0, beforeHp - remaining);
        player.MaxHp = maxHp;
        onRosterDirty?.Invoke();
        ScheduleVitalsMirror(accountId, characterName, player);

        var hpWire = (ushort)Math.Clamp(player.CurrentHp, 0, ushort.MaxValue);
        var sdWire = (ushort)Math.Clamp(player.CurrentShield, 0, ushort.MaxValue);
        await writeAsync(LifeManaWire602.BuildLife(LifeManaWire602.TypeCurrent, hpWire, sdWire), ct).ConfigureAwait(false);

        if (player.CurrentHp <= 0 && playerObjectKey != 0)
        {
            await writeAsync(PlayerDieWire602.Build(playerObjectKey, killerObjectKey), ct).ConfigureAwait(false);
        }

        return true;
    }

    public static void ScheduleVitalsMirror(string? accountId, string? characterName, GameRosterEntry player) =>
        ScheduleVitalsMirror(
            accountId,
            characterName,
            player.CurrentHp,
            player.MaxHp,
            player.CurrentMp,
            player.MaxMp,
            player.CurrentShield,
            player.MaxShield);

    public static void ScheduleVitalsMirror(
        string? accountId,
        string? characterName,
        int currentHp,
        int maxHp,
        int currentMp,
        int maxMp,
        int currentShield = 0,
        int maxShield = 0)
    {
        if (string.IsNullOrEmpty(accountId) || string.IsNullOrWhiteSpace(characterName))
        {
            return;
        }

        CharacterRosterMirrorWriter.ScheduleVitalsUpsert(
            accountId,
            characterName,
            currentHp,
            maxHp,
            currentMp,
            maxMp,
            currentShield,
            maxShield);
    }
}
