using System.Collections.Concurrent;

namespace Takumi.Server.Game.World;

/// <summary>Per-map dropped items (parity <c>gMap[].m_Item</c> simplified).</summary>
public static class MapGroundItemStore
{
    const int MaxIndex = 0x7FFE;

    static readonly ConcurrentDictionary<byte, ConcurrentDictionary<ushort, GroundItem>> ByMap = new();

    public sealed record GroundItem(ushort Index, byte X, byte Y, byte[] Item12);

    public static ushort Drop(byte mapId, byte x, byte y, byte[] item12)
    {
        var map = ByMap.GetOrAdd(mapId, _ => new ConcurrentDictionary<ushort, GroundItem>());
        for (ushort i = 1; i <= MaxIndex; i++)
        {
            if (!map.ContainsKey(i))
            {
                map[i] = new GroundItem(i, x, y, item12);
                return i;
            }
        }

        return 0;
    }

    public static bool TryPeek(byte mapId, ushort index, out GroundItem? item)
    {
        item = null;
        if (index == 0 || index > MaxIndex)
        {
            return false;
        }

        return ByMap.TryGetValue(mapId, out var map) && map.TryGetValue(index, out item);
    }

    public static bool TryTake(byte mapId, ushort index, byte playerX, byte playerY, out byte[] item12)
    {
        item12 = Array.Empty<byte>();
        if (!TryPeek(mapId, index, out var ground) || ground is null)
        {
            return false;
        }

        if (!IsWithinPickupRange(playerX, playerY, ground.X, ground.Y))
        {
            return false;
        }

        if (!ByMap.TryGetValue(mapId, out var map) || !map.TryRemove(index, out _))
        {
            return false;
        }

        item12 = ground.Item12;
        return true;
    }

    public static bool IsWithinPickupRange(byte px, byte py, byte ix, byte iy)
    {
        var dx = Math.Abs(px - ix);
        var dy = Math.Abs(py - iy);
        return dx <= 2 && dy <= 2;
    }

    public static void ClearMap(byte mapId) => ByMap.TryRemove(mapId, out _);
}
