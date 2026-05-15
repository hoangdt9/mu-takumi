namespace Takumi.Server.Game.World;

public sealed class MapMonsterInstance
{
    public required int ObjectKey { get; init; }
    public required int MonsterClass { get; init; }
    public required byte Map { get; init; }
    public required byte X { get; init; }
    public required byte Y { get; init; }
    public required byte Dir { get; init; }
    public required int Life { get; init; }
    public required int Level { get; init; }
    public required int RegenDelayMs { get; init; }

    public bool IsAlive { get; private set; } = true;
    DateTimeOffset? _diedAtUtc;

    public void MarkDead()
    {
        if (!IsAlive)
        {
            return;
        }

        IsAlive = false;
        _diedAtUtc = DateTimeOffset.UtcNow;
    }

    /// <summary>Respawn at spawn tile when regen delay elapsed (parity <c>gObjMonsterRegen</c>).</summary>
    public bool TryRegen()
    {
        if (IsAlive)
        {
            return false;
        }

        if (_diedAtUtc is null)
        {
            IsAlive = true;
            return true;
        }

        var elapsed = DateTimeOffset.UtcNow - _diedAtUtc.Value;
        if (elapsed.TotalMilliseconds < RegenDelayMs)
        {
            return false;
        }

        IsAlive = true;
        _diedAtUtc = null;
        return true;
    }
}
