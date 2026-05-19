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

    readonly Dictionary<int, int> _damageByPlayerKey = new();

    public int CurrentLife { get; private set; }
    public bool IsAlive { get; private set; } = true;
    DateTimeOffset? _diedAtUtc;
    byte? _deathX;
    byte? _deathY;

    public void InitializeAtSpawn(byte x, byte y, byte dir)
    {
        X = x;
        Y = y;
        Dir = dir;
        InitializeLife();
    }

    public void InitializeLife()
    {
        CurrentLife = MaxLife;
        ClearDamageLedger();
    }

    public void RecordHit(int playerObjectKey, int amount)
    {
        if (IsNpc || !IsAlive || amount <= 0 || playerObjectKey <= 0)
        {
            return;
        }

        _damageByPlayerKey[playerObjectKey] = _damageByPlayerKey.GetValueOrDefault(playerObjectKey) + amount;
    }

    public bool TryGetTopDamagePlayerKey(out int playerObjectKey, out int totalDamage)
    {
        playerObjectKey = 0;
        totalDamage = 0;
        foreach (var (key, dmg) in _damageByPlayerKey)
        {
            if (dmg <= totalDamage)
            {
                continue;
            }

            totalDamage = dmg;
            playerObjectKey = key;
        }

        return totalDamage > 0;
    }

    public void ClearDamageLedger() => _damageByPlayerKey.Clear();

    public int TotalRecordedDamage()
    {
        var sum = 0;
        foreach (var dmg in _damageByPlayerKey.Values)
        {
            sum += dmg;
        }

        return sum;
    }

    public void CopyDamageContributors(List<(int PlayerKey, int Damage)> into)
    {
        into.Clear();
        foreach (var (key, dmg) in _damageByPlayerKey)
        {
            if (dmg > 0)
            {
                into.Add((key, dmg));
            }
        }
    }

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
        _deathX = X;
        _deathY = Y;
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
            RegenAtResolvedTile();
            return true;
        }

        var elapsed = DateTimeOffset.UtcNow - _diedAtUtc.Value;
        if (elapsed.TotalMilliseconds < RegenDelayMs)
        {
            return false;
        }

        RegenAtResolvedTile();
        return true;
    }

    public void ResetToSpawn()
    {
        IsAlive = true;
        CurrentLife = MaxLife;
        _diedAtUtc = null;
        _deathX = null;
        _deathY = null;
        X = SpawnX;
        Y = SpawnY;
        AggroTargetKey = null;
        ClearDamageLedger();
    }

    void RegenAtResolvedTile()
    {
        if (KalimaMapRegen.IsKalimaMap(Map)
            && _deathX is byte dx
            && _deathY is byte dy
            && MapAttWalkability.CanWalk(Map, dx, dy))
        {
            RegenAt(dx, dy);
            return;
        }

        ResetToSpawn();
    }

    void RegenAt(byte x, byte y)
    {
        IsAlive = true;
        CurrentLife = MaxLife;
        _diedAtUtc = null;
        _deathX = null;
        _deathY = null;
        X = x;
        Y = y;
        AggroTargetKey = null;
        ClearDamageLedger();
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

        var pathSteps = ParseIntEnv("TAKUMI_MONSTER_PATHFIND_MAX_STEPS", 12, 1, 32);
        if (MapTilePathfinder.TryFindNextStep(mapId, X, Y, targetX, targetY, pathSteps, out var px, out var py)
            && IsWithinLeash(px, py))
        {
            nextX = px;
            nextY = py;
            nextDir = DirectionFromDelta((int)px - X, (int)py - Y);
            X = nextX;
            Y = nextY;
            Dir = nextDir;
            return true;
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

            if (!IsWithinLeash((byte)nx, (byte)ny))
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

            if (!IsWithinLeash((byte)nx, (byte)ny))
            {
                continue;
            }

            if (!MapAttWalkability.CanWalk(mapId, (byte)nx, (byte)ny))
            {
                continue;
            }

            if (MapAttWalkability.IsSafeZone(mapId, (byte)nx, (byte)ny))
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

    bool IsWithinLeash(byte x, byte y)
    {
        var leash = Math.Max(WanderLeash, MoveRange);
        return Math.Abs(x - SpawnX) + Math.Abs(y - SpawnY) <= leash;
    }

    static int ParseIntEnv(string key, int defaultValue, int min, int max)
    {
        var raw = Environment.GetEnvironmentVariable(key);
        if (string.IsNullOrWhiteSpace(raw)
            || !int.TryParse(raw, System.Globalization.NumberStyles.Integer, System.Globalization.CultureInfo.InvariantCulture, out var v))
        {
            return defaultValue;
        }

        return Math.Clamp(v, min, max);
    }
}
