using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MapMonsterCombatResolveTests
{
    [Fact]
    public void TryResolveCombatTarget_falls_back_to_nearest_field_mob_when_wire_key_stale()
    {
        var near = CreateMob(12_001, x: 10, y: 10);
        var far = CreateMob(12_002, x: 20, y: 20);
        InjectForTest(mapId: 3, near, far);

        Assert.True(
            MapMonsterWorld.TryResolveCombatTarget(3, 10, 11, wireTargetId: 5911, meleeRange: 3, out var resolved));
        Assert.NotNull(resolved);
        Assert.Equal(12_001, resolved!.ObjectKey);
    }

    [Fact]
    public void TryResolveCombatTarget_uses_exact_key_when_present()
    {
        var mob = CreateMob(12_050, x: 5, y: 5);
        InjectForTest(mapId: 0, mob);

        Assert.True(MapMonsterWorld.TryResolveCombatTarget(0, 5, 5, 12_050, 3, out var resolved));
        Assert.Equal(12_050, resolved!.ObjectKey);
    }

    static MapMonsterInstance CreateMob(int key, byte x, byte y)
    {
        var inst = new MapMonsterInstance
        {
            ObjectKey = key,
            MonsterClass = 3,
            Map = 3,
            SpawnX = x,
            SpawnY = y,
            WanderLeash = 3,
            MoveRange = 3,
            MaxLife = 100,
            Level = 5,
            RegenDelayMs = 10_000,
            IsNpc = false,
        };
        inst.InitializeAtSpawn(x, y, 0);
        return inst;
    }

    static void InjectForTest(byte mapId, params MapMonsterInstance[] mobs)
    {
        Environment.SetEnvironmentVariable("TAKUMI_MONSTER_SET_BASE_PATH", string.Empty);
        Environment.SetEnvironmentVariable("TAKUMI_MONSTER_INFO_PATH", string.Empty);
        MapMonsterWorld.EnsureInitialized();
        var field = typeof(MapMonsterWorld).GetField("_byMap", System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic);
        var byKey = typeof(MapMonsterWorld).GetField("_byObjectKey", System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic);
        Assert.NotNull(field);
        Assert.NotNull(byKey);
        var list = mobs.ToList();
        field!.SetValue(null, new Dictionary<byte, List<MapMonsterInstance>> { [mapId] = list });
        byKey!.SetValue(null, list.ToDictionary(m => m.ObjectKey));
    }
}
