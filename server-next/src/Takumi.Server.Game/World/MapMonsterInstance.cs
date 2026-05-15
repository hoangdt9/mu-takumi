namespace Takumi.Server.Game.World;

public sealed class MapMonsterInstance
{
    static readonly (int Dx, int Dy)[] WanderDeltas =
    [
        (0, -1),
        (1, -1),
        (1, 0),
        (1, 1),
        (0, 1),
        (-1, 1),
        (-1, 0),
        (-1, -1),
    ];

    public required int ObjectKey { get; init; }
    public required int MonsterClass { get; init; }
    public required byte Map { get; init; }
    public required byte SpawnX { get; init; }
    public required byte SpawnY { get; init; }
    public required byte WanderLeash { get; init; }
    public required byte MoveRange { get; init; }
    public required int MaxLife { get; init; }
    public required int Level { get; init; }
    public required int RegenDelayMs { get; init; }
    public bool IsNpc { get; init; }

    public byte X { get; private set; }
    public byte Y { get; private set; }
    public byte Dir { get; private set; }

    public int? AggroTargetKey { get; private set; }

    public int CurrentLife { get; private set; }
    public bool IsAlive { get; private set; } = true;
    DateTimeOffset? _diedAtUtc;

    public void InitializeAtSpawn(byte x, byte y, byte dir)
    {
        X = x;
        Y = y;
        Dir = dir;
        InitializeLife();
    }

    public void InitializeLife() => CurrentLife = MaxLife;

    /// <returns><see langword="true"/> if the monster died from this hit.</returns>
    public bool ApplyDamage(int amount)
    {
        if (IsNpc || !IsAlive || amount <= 0)
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
            ResetToSpawn();
            return true;
        }

        var elapsed = DateTimeOffset.UtcNow - _diedAtUtc.Value;
        if (elapsed.TotalMilliseconds < RegenDelayMs)
        {
            return false;
        }

        ResetToSpawn();
        return true;
    }

    public void ResetToSpawn()
    {
        IsAlive = true;
        CurrentLife = MaxLife;
        _diedAtUtc = null;
        X = SpawnX;
        Y = SpawnY;
        AggroTargetKey = null;
    }

    public void SetAggro(int targetObjectKey) => AggroTargetKey = targetObjectKey;

    public void ClearAggro() => AggroTargetKey = null;

    public int ManhattanTo(byte px, byte py) => Math.Abs(X - px) + Math.Abs(Y - py);

    /// <summary>Greedy one-step chase (parity <c>PathFinding</c> stub before full A*).</summary>
    public bool TryChaseStep(byte targetX, byte targetY, byte mapId, out byte nextX, out byte nextY, out byte nextDir)
    {
        nextX = X;
        nextY = Y;
        nextDir = Dir;
        if (IsNpc || !IsAlive)
        {
            return false;
        }

        var bestDist = ManhattanTo(targetX, targetY);
        if (bestDist <= 0)
        {
            return false;
        }

        var found = false;
        var bestNext = bestDist;
        byte bx = X, by = Y, bdir = Dir;
        foreach (var (dx, dy) in WanderDeltas)
        {
            var nx = X + dx;
            var ny = Y + dy;
            if (nx is < 0 or > 255 || ny is < 0 or > 255)
            {
                continue;
            }

            var leash = Math.Max(WanderLeash, MoveRange);
            if (Math.Abs(nx - SpawnX) + Math.Abs(ny - SpawnY) > leash)
            {
                continue;
            }

            if (!MapAttWalkability.CanWalk(mapId, (byte)nx, (byte)ny))
            {
                continue;
            }

            var dist = Math.Abs(nx - targetX) + Math.Abs(ny - targetY);
            if (dist >= bestNext)
            {
                continue;
            }

            bestNext = dist;
            bx = (byte)nx;
            by = (byte)ny;
            bdir = DirectionFromDelta(dx, dy);
            found = true;
        }

        if (!found)
        {
            return false;
        }

        nextX = bx;
        nextY = by;
        nextDir = bdir;
        X = nextX;
        Y = nextY;
        Dir = nextDir;
        return true;
    }

    /// <summary>Pick a random adjacent tile within spawn leash (parity <c>MonsterMoveCheck</c>).</summary>
    public bool TryRollWander(Random rng, byte mapId, out byte nextX, out byte nextY, out byte nextDir)
    {
        nextX = X;
        nextY = Y;
        nextDir = Dir;
        if (IsNpc || !IsAlive)
        {
            return false;
        }

        var leash = Math.Max(WanderLeash, MoveRange);
        if (leash <= 0)
        {
            return false;
        }

        var start = rng.Next(0, WanderDeltas.Length);
        for (var i = 0; i < WanderDeltas.Length; i++)
        {
            var (dx, dy) = WanderDeltas[(start + i) % WanderDeltas.Length];
            var nx = X + dx;
            var ny = Y + dy;
            if (nx is < 0 or > 255 || ny is < 0 or > 255)
            {
                continue;
            }

            if (Math.Abs(nx - SpawnX) + Math.Abs(ny - SpawnY) > leash)
            {
                continue;
            }

            if (!MapAttWalkability.CanWalk(mapId, (byte)nx, (byte)ny))
            {
                continue;
            }

            nextX = (byte)nx;
            nextY = (byte)ny;
            nextDir = DirectionFromDelta(dx, dy);
            X = nextX;
            Y = nextY;
            Dir = nextDir;
            return true;
        }

        return false;
    }

    static byte DirectionFromDelta(int dx, int dy)
    {
        if (dx == 0 && dy < 0)
        {
            return 0;
        }

        if (dx > 0 && dy < 0)
        {
            return 1;
        }

        if (dx > 0 && dy == 0)
        {
            return 2;
        }

        if (dx > 0 && dy > 0)
        {
            return 3;
        }

        if (dx == 0 && dy > 0)
        {
            return 4;
        }

        if (dx < 0 && dy > 0)
        {
            return 5;
        }

        if (dx < 0 && dy == 0)
        {
            return 6;
        }

        return 7;
    }
}
