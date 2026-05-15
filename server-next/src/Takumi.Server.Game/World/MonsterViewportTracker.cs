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
        if (inView.Count == 0)
        {
            return Array.Empty<MapMonsterInstance>();
        }

        var fresh = new List<MapMonsterInstance>();
        foreach (var m in inView)
        {
            if (_sentKeys.Add(m.ObjectKey))
            {
                fresh.Add(m);
            }
        }

        return fresh;
    }
}
