namespace Takumi.Server.Game.World;

public sealed class MapMonsterInstance
{
    public required int ObjectKey { get; init; }
    public required int MonsterClass { get; init; }
    public required byte Map { get; init; }
    public required byte X { get; init; }
    public required byte Y { get; init; }
    public required byte Dir { get; init; }
    public required int MaxLife { get; init; }
    public required int Level { get; init; }
    public required int RegenDelayMs { get; init; }

    public int CurrentLife { get; private set; }
    public bool IsAlive { get; private set; } = true;
    DateTimeOffset? _diedAtUtc;

    public void InitializeLife() => CurrentLife = MaxLife;

    /// <returns><see langword="true"/> if the monster died from this hit.</returns>
    public bool ApplyDamage(int amount)
    {
        if (!IsAlive || amount <= 0)
        {
            return false;
        }

        CurrentLife = Math.Max(0, CurrentLife - amount);
        if (CurrentLife > 0)
        {
            return false;
        }

        MarkDead();
        return true;
    }

    public void MarkDead()
    {
        if (!IsAlive)
        {
            return;
        }

        IsAlive = false;
        CurrentLife = 0;
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
            CurrentLife = MaxLife;
            return true;
        }

        var elapsed = DateTimeOffset.UtcNow - _diedAtUtc.Value;
        if (elapsed.TotalMilliseconds < RegenDelayMs)
        {
            return false;
        }

        IsAlive = true;
        CurrentLife = MaxLife;
        _diedAtUtc = null;
        return true;
    }
}
