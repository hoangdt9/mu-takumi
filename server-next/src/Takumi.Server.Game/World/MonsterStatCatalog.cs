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

        foreach (var raw in File.ReadAllLines(path))
        {
            var line = raw.Trim();
            if (line.Length == 0 || line.StartsWith("//", StringComparison.Ordinal))
            {
                continue;
            }

            if (string.Equals(line, "end", StringComparison.OrdinalIgnoreCase))
            {
                break;
            }

            var parts = line.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length < 4)
            {
                continue;
            }

            var index = int.Parse(parts[0], CultureInfo.InvariantCulture);
            var level = int.Parse(parts[3], CultureInfo.InvariantCulture);
            var life = int.Parse(parts[4], CultureInfo.InvariantCulture);
            var regenSeconds = parts.Length > 18
                ? int.Parse(parts[18], CultureInfo.InvariantCulture)
                : 10;
            cat._byClass[index] = new MonsterStat(index, level, life, regenSeconds);
        }

        return cat;
    }

    public MonsterStat GetOrDefault(int monsterClass) =>
        _byClass.TryGetValue(monsterClass, out var s)
            ? s
            : new MonsterStat(monsterClass, Level: 1, Life: 100, RegenTimeSeconds: 10);
}

public readonly record struct MonsterStat(int Index, int Level, int Life, int RegenTimeSeconds);
