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
        var moveRange = col + 12 < parts.Length
            && int.TryParse(parts[col + 12], NumberStyles.Integer, CultureInfo.InvariantCulture, out var mr)
            ? mr
            : 3;
        var regenCol = col + 17;
        var regenSeconds = regenCol < parts.Length
            && int.TryParse(parts[regenCol], NumberStyles.Integer, CultureInfo.InvariantCulture, out var regen)
            ? regen
            : 10;

        stat = new MonsterStat(index, level, life, damageMin, damageMax, defense, moveRange, regenSeconds);
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
                RegenTimeSeconds: 10);
}

public readonly record struct MonsterStat(
    int Index,
    int Level,
    int Life,
    int DamageMin,
    int DamageMax,
    int Defense,
    int MoveRange,
    int RegenTimeSeconds);
