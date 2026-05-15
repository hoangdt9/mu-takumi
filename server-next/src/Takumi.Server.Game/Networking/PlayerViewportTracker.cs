namespace Takumi.Server.Game.Networking;

/// <summary>Per-session peer player keys already sent via <c>C2 0x12</c>.</summary>
public sealed class PlayerViewportTracker
{
    readonly HashSet<int> _visiblePeerKeys = new();
    byte _map;
    byte _anchorX;
    byte _anchorY;
    bool _hasAnchor;

    public void ResetForMap(byte mapId, byte anchorX, byte anchorY)
    {
        _visiblePeerKeys.Clear();
        _map = mapId;
        _anchorX = anchorX;
        _anchorY = anchorY;
        _hasAnchor = true;
    }

    public void Clear() => _visiblePeerKeys.Clear();

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

    public (IReadOnlyList<int> Entered, IReadOnlyList<int> Left) SyncPeers(IReadOnlyList<int> inRangeKeys)
    {
        var inRange = new HashSet<int>(inRangeKeys);
        var left = new List<int>();
        foreach (var key in _visiblePeerKeys.ToArray())
        {
            if (inRange.Contains(key))
            {
                continue;
            }

            left.Add(key);
            _visiblePeerKeys.Remove(key);
        }

        var entered = new List<int>();
        foreach (var key in inRange)
        {
            if (_visiblePeerKeys.Add(key))
            {
                entered.Add(key);
            }
        }

        return (entered, left);
    }

    public bool TryMarkVisible(int peerKey) => _visiblePeerKeys.Add(peerKey);

    public bool TryForget(int peerKey) => _visiblePeerKeys.Remove(peerKey);
}
