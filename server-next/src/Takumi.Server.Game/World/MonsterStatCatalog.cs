using System.Globalization;

namespace Takumi.Server.Game.World;

/// <summary>Minimal <c>Monster.txt</c> stats (parity <c>CMonsterManager::Load</c> — Life/Level used for M9A).</summary>
public sealed class MonsterStatCatalog
{
    readonly Dictionary<int, MonsterStat> _byClass = new();

    public static MonsterStatCatalog LoadFromFile(string path)
    {
        var cat = new MonsterStatCatalog();
        if (!File.Exists(path))
        {
            return cat;
        }

        var loaded = 0;
        var skipped = 0;
        foreach (var parts in GameDataTextTableLoader.ReadDataRows(path))
        {
            if (!TryParseRow(parts, out var stat))
            {
                skipped++;
                continue;
            }

            cat._byClass[stat.Index] = stat;
            loaded++;
        }

        Console.WriteLine("[m9] loaded Monster.txt: {0} classes ({1} rows skipped) from {2}", loaded, skipped, path);
        return cat;
    }

    /// <summary>Parses one data row; supports quoted names with spaces (<c>"Bull Fighter"</c>).</summary>
    internal static bool TryParseRow(string[] parts, out MonsterStat stat)
    {
        stat = default;
        if (parts.Length < 8 || !int.TryParse(parts[0], NumberStyles.Integer, CultureInfo.InvariantCulture, out var index))
        {
            return false;
        }

        var col = 2;
        if (col >= parts.Length)
        {
            return false;
        }

        if (parts[col].StartsWith("\"", StringComparison.Ordinal))
        {
            if (parts[col].EndsWith("\"", StringComparison.Ordinal) && parts[col].Length > 1)
            {
                col++;
            }
            else
            {
                while (col < parts.Length && !parts[col].EndsWith("\"", StringComparison.Ordinal))
                {
                    col++;
                }

                col++;
            }
        }
        else
        {
            col++;
        }

        if (col + 5 >= parts.Length)
        {
            return false;
        }

        if (!int.TryParse(parts[col], NumberStyles.Integer, CultureInfo.InvariantCulture, out var level)
            || !int.TryParse(parts[col + 1], NumberStyles.Integer, CultureInfo.InvariantCulture, out var life))
        {
            return false;
        }

        var damageMin = col + 3 < parts.Length
            && int.TryParse(parts[col + 3], NumberStyles.Integer, CultureInfo.InvariantCulture, out var dmin)
            ? dmin
            : 0;
        var damageMax = col + 4 < parts.Length
            && int.TryParse(parts[col + 4], NumberStyles.Integer, CultureInfo.InvariantCulture, out var dmax)
            ? dmax
            : damageMin;
        var defense = col + 5 < parts.Length
            && int.TryParse(parts[col + 5], NumberStyles.Integer, CultureInfo.InvariantCulture, out var def)
            ? def
            : 0;
        var moveRange = col + 9 < parts.Length
            && int.TryParse(parts[col + 9], NumberStyles.Integer, CultureInfo.InvariantCulture, out var mr)
            ? mr
            : 3;
        var attackType = col + 10 < parts.Length
            && int.TryParse(parts[col + 10], NumberStyles.Integer, CultureInfo.InvariantCulture, out var at)
            ? at
            : 0;
        var attackRange = col + 11 < parts.Length
            && int.TryParse(parts[col + 11], NumberStyles.Integer, CultureInfo.InvariantCulture, out var ar)
            ? ar
            : 1;
        var viewRange = col + 12 < parts.Length
            && int.TryParse(parts[col + 12], NumberStyles.Integer, CultureInfo.InvariantCulture, out var vr)
            ? vr
            : 5;
        var regenCol = col + 15;
        var regenSeconds = regenCol < parts.Length
            && int.TryParse(parts[regenCol], NumberStyles.Integer, CultureInfo.InvariantCulture, out var regen)
            ? regen
            : 10;
        var attribute = col + 16 < parts.Length
            && int.TryParse(parts[col + 16], NumberStyles.Integer, CultureInfo.InvariantCulture, out var attr)
            ? attr
            : 0;
        var resist0 = col + 21 < parts.Length
            && int.TryParse(parts[col + 21], NumberStyles.Integer, CultureInfo.InvariantCulture, out var r0)
            ? Math.Clamp(r0, 0, 255)
            : 0;
        var resist1 = col + 22 < parts.Length
            && int.TryParse(parts[col + 22], NumberStyles.Integer, CultureInfo.InvariantCulture, out var r1)
            ? Math.Clamp(r1, 0, 255)
            : 0;
        var resist2 = col + 23 < parts.Length
            && int.TryParse(parts[col + 23], NumberStyles.Integer, CultureInfo.InvariantCulture, out var r2)
            ? Math.Clamp(r2, 0, 255)
            : 0;
        var resist3 = col + 24 < parts.Length
            && int.TryParse(parts[col + 24], NumberStyles.Integer, CultureInfo.InvariantCulture, out var r3)
            ? Math.Clamp(r3, 0, 255)
            : 0;
        var elementalAttribute = 0;
        var elementalDefense = 0;
        if (col + 25 < parts.Length
            && int.TryParse(parts[col + 25], NumberStyles.Integer, CultureInfo.InvariantCulture, out var elemAttr))
        {
            elementalAttribute = elemAttr;
            if (col + 27 < parts.Length
                && int.TryParse(parts[col + 27], NumberStyles.Integer, CultureInfo.InvariantCulture, out var elemDef))
            {
                elementalDefense = Math.Max(0, elemDef);
            }
        }

        stat = new MonsterStat(
            index,
            level,
            life,
            damageMin,
            damageMax,
            defense,
            moveRange,
            attackType,
            attackRange,
            viewRange,
            regenSeconds,
            attribute,
            resist0,
            resist1,
            resist2,
            resist3,
            elementalAttribute,
            elementalDefense);
        return true;
    }

    public MonsterStat GetOrDefault(int monsterClass) =>
        _byClass.TryGetValue(monsterClass, out var s)
            ? s
            : new MonsterStat(
                monsterClass,
                Level: 1,
                Life: 100,
                DamageMin: 5,
                DamageMax: 10,
                Defense: 0,
                MoveRange: 3,
                AttackType: 0,
                AttackRange: 1,
                ViewRange: 5,
                RegenTimeSeconds: 10,
                Attribute: 0,
                Resistance0: 0,
                Resistance1: 0,
                Resistance2: 0,
                Resistance3: 0,
                ElementalAttribute: 0,
                ElementalDefense: 0);
}

public readonly record struct MonsterStat(
    int Index,
    int Level,
    int Life,
    int DamageMin,
    int DamageMax,
    int Defense,
    int MoveRange,
    int AttackType,
    int AttackRange,
    int ViewRange,
    int RegenTimeSeconds,
    int Attribute = 0,
    int Resistance0 = 0,
    int Resistance1 = 0,
    int Resistance2 = 0,
    int Resistance3 = 0,
    int ElementalAttribute = 0,
    int ElementalDefense = 0)
{
    public bool UsesRangedOrMagic => AttackType >= 100;
}
