namespace Takumi.Server.Game.World;

/// <summary>Per-session monster keys already sent via <c>C2 0x13</c> (parity viewport state).</summary>
public sealed class MonsterViewportTracker
{
    readonly HashSet<int> _sentKeys = new();
    byte _map;
    byte _anchorX;
    byte _anchorY;
    bool _hasAnchor;

    public void ResetForMap(byte mapId, byte anchorX, byte anchorY)
    {
        _sentKeys.Clear();
        _map = mapId;
        _anchorX = anchorX;
        _anchorY = anchorY;
        _hasAnchor = true;
    }

    public void Clear() => _sentKeys.Clear();

    public void Forget(int objectKey) => _sentKeys.Remove(objectKey);

    public bool ShouldRescan(byte mapId, byte playerX, byte playerY, int moveThresholdTiles)
    {
        if (!_hasAnchor || mapId != _map)
        {
            return true;
        }

        return Math.Abs(playerX - _anchorX) + Math.Abs(playerY - _anchorY) >= moveThresholdTiles;
    }

    public void NoteAnchor(byte mapId, byte playerX, byte playerY)
    {
        _map = mapId;
        _anchorX = playerX;
        _anchorY = playerY;
        _hasAnchor = true;
    }

    public IReadOnlyList<MapMonsterInstance> TakeNewInView(IReadOnlyList<MapMonsterInstance> inView)
    {
        return SyncView(inView).Entered;
    }

    /// <summary>Diff viewport: new spawns to create (<c>0x13</c>) and keys to destroy (<c>0x14</c>).</summary>
    public (IReadOnlyList<MapMonsterInstance> Entered, IReadOnlyList<int> LeftKeys) SyncView(
        IReadOnlyList<MapMonsterInstance> inView)
    {
        var inRange = new HashSet<int>();
        foreach (var m in inView)
        {
            inRange.Add(m.ObjectKey);
        }

        var left = new List<int>();
        var destroyNpcs = string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_MONSTER_VIEWPORT_DESTROY_NPC")?.Trim(),
            "1",
            StringComparison.OrdinalIgnoreCase);
        foreach (var key in _sentKeys.ToArray())
        {
            if (inRange.Contains(key))
            {
                continue;
            }

            if (!destroyNpcs
                && MapMonsterWorld.TryGetMonster(key, out var mob)
                && mob is { IsNpc: true })
            {
                continue;
            }

            left.Add(key);
            _sentKeys.Remove(key);
        }

        var entered = new List<MapMonsterInstance>();
        foreach (var m in inView)
        {
            if (_sentKeys.Add(m.ObjectKey))
            {
                entered.Add(m);
            }
        }

        return (entered, left);
    }
}
