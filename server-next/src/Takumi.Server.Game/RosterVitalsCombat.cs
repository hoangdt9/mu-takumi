using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>M7d: apply player HP/MP changes and emit <c>0x26</c>/<c>0x27</c>.</summary>
public static class RosterVitalsCombat
{
    public static async Task<bool> ApplyPlayerDamageAsync(
        GameRosterEntry player,
        int damage,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        CancellationToken ct)
    {
        if (damage <= 0 || player.MaxHp <= 0)
        {
            return false;
        }

        var maxHp = Math.Max(1, player.MaxHp);
        var before = player.CurrentHp > 0 ? player.CurrentHp : maxHp;
        player.CurrentHp = Math.Max(0, before - damage);
        player.MaxHp = maxHp;
        onRosterDirty?.Invoke();

        var hpWire = (ushort)Math.Clamp(player.CurrentHp, 0, ushort.MaxValue);
        await writeAsync(LifeManaWire602.BuildLife(LifeManaWire602.TypeCurrent, hpWire), ct).ConfigureAwait(false);
        return true;
    }

    public static void SyncMonsterViewerHp(Guid sessionId, GameRosterEntry player)
    {
        if (Networking.MonsterViewerRegistry.TryGetSession(sessionId, out var session))
        {
            session.CurrentHp = player.CurrentHp;
            session.MaxHp = Math.Max(1, player.MaxHp);
        }
    }
}
