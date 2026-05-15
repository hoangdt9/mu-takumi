namespace Takumi.Server.Game.World;

/// <summary>Resolves tile from set-base row (parity <c>CMonsterSetBase::GetPosition</c>).</summary>
public static class MonsterSpawnResolver
{
    public static bool TryResolvePosition(MonsterSetBaseEntry entry, out byte x, out byte y)
    {
        x = 0;
        y = 0;
        if (entry.SpawnType is 0 or 4)
        {
            x = (byte)entry.X;
            y = (byte)entry.Y;
            return true;
        }

        if (entry.SpawnType is 1 or 3)
        {
            return TryBoxPosition(entry.X, entry.Y, entry.Tx, entry.Ty, out x, out y);
        }

        if (entry.SpawnType == 2)
        {
            return TryBoxPosition(entry.X - 3, entry.Y - 3, entry.X + 3, entry.Y + 3, out x, out y);
        }

        return false;
    }

    static bool TryBoxPosition(int x, int y, int tx, int ty, out byte ox, out byte oy)
    {
        ox = 0;
        oy = 0;
        for (var n = 0; n < 100; n++)
        {
            var subx = Math.Max(1, tx - x);
            var suby = Math.Max(1, ty - y);
            var rx = x + Random.Shared.Next(subx);
            var ry = y + Random.Shared.Next(suby);
            if (rx is < 0 or > 255 || ry is < 0 or > 255)
            {
                continue;
            }

            ox = (byte)rx;
            oy = (byte)ry;
            return true;
        }

        return false;
    }
}
