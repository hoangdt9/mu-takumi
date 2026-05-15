namespace Takumi.Server.Game.World;

/// <summary>Greedy path toward target on ATT (parity <c>CMap::PathFinding2</c> stub — first step on improving route).</summary>
public static class MapTilePathfinder
{
    static readonly (int Dx, int Dy)[] Deltas =
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

    /// <summary>Returns the first step on a path from (sx,sy) toward (tx,ty).</summary>
    public static bool TryFindNextStep(
        byte mapId,
        byte sx,
        byte sy,
        byte tx,
        byte ty,
        int maxSteps,
        out byte nextX,
        out byte nextY)
    {
        nextX = sx;
        nextY = sy;
        if (sx == tx && sy == ty)
        {
            return false;
        }

        maxSteps = Math.Clamp(maxSteps, 1, 32);
        var cx = (int)sx;
        var cy = (int)sy;
        var startDist = Manhattan(cx, cy, tx, ty);

        for (var step = 0; step < maxSteps; step++)
        {
            if (cx == tx && cy == ty)
            {
                return step > 0;
            }

            var bestX = cx;
            var bestY = cy;
            var bestDist = startDist;
            foreach (var (dx, dy) in Deltas)
            {
                var nx = cx + dx;
                var ny = cy + dy;
                if (nx is < 0 or > 255 || ny is < 0 or > 255)
                {
                    continue;
                }

                var isGoal = nx == tx && ny == ty;
                if (!isGoal && !MapAttWalkability.CanWalk(mapId, (byte)nx, (byte)ny))
                {
                    continue;
                }

                var dist = Manhattan(nx, ny, tx, ty);
                if (dist >= bestDist)
                {
                    continue;
                }

                bestDist = dist;
                bestX = nx;
                bestY = ny;
            }

            if (bestX == cx && bestY == cy)
            {
                return step > 0;
            }

            if (step == 0)
            {
                nextX = (byte)bestX;
                nextY = (byte)bestY;
            }

            cx = bestX;
            cy = bestY;
            if (bestDist < startDist)
            {
                return true;
            }
        }

        return nextX != sx || nextY != sy;
    }

    static int Manhattan(int x, int y, byte tx, byte ty) => Math.Abs(x - tx) + Math.Abs(y - ty);
}
