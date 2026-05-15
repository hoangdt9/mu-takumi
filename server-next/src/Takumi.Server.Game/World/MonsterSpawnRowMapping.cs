using Takumi.Server.Persistence;

namespace Takumi.Server.Game.World;

public static class MonsterSpawnRowMapping
{
    public static MonsterSpawnRow ToRow(MonsterSetBaseEntry e) =>
        new()
        {
            SpawnType = (short)e.SpawnType,
            MonsterClass = e.MonsterClass,
            MapId = e.Map,
            Dis = e.Dis,
            PosX = (short)e.X,
            PosY = (short)e.Y,
            RangeTx = (short)e.Tx,
            RangeTy = (short)e.Ty,
            Dir = e.Dir,
            SpawnValue = e.Value,
        };

    public static MonsterSetBaseEntry FromRow(MonsterSpawnRow r) =>
        new()
        {
            SpawnType = r.SpawnType,
            MonsterClass = r.MonsterClass,
            Map = r.MapId,
            Dis = r.Dis,
            X = r.PosX,
            Y = r.PosY,
            Tx = r.RangeTx,
            Ty = r.RangeTy,
            Dir = (byte)r.Dir,
            Value = r.SpawnValue,
        };
}
