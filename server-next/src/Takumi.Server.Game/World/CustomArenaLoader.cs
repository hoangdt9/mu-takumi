namespace Takumi.Server.Game.World;

/// <summary>Loads <c>Custom/CustomArena.txt</c> (parity <c>CCustomArena::Load</c>).</summary>
public static class CustomArenaLoader
{
    public sealed record LoadResult(
        IReadOnlyList<CustomArenaStartTimeEntry> Schedules,
        IReadOnlyList<CustomArenaRuleEntry> Rules);

    public static LoadResult LoadFromFile(string path)
    {
        if (!File.Exists(path))
        {
            throw new FileNotFoundException("CustomArena file not found.", path);
        }

        var schedules = new List<CustomArenaStartTimeEntry>(16);
        var rules = new List<CustomArenaRuleEntry>(8);
        var section = -1;

        foreach (var raw in File.ReadAllLines(path))
        {
            var line = GameDataTextTableLoader.StripComment(raw).Trim();
            if (line.Length == 0 || line.StartsWith("//", StringComparison.Ordinal))
            {
                continue;
            }

            if (line.Length == 1 && char.IsDigit(line[0]))
            {
                section = line[0] - '0';
                continue;
            }

            if (string.Equals(line, "end", StringComparison.OrdinalIgnoreCase))
            {
                if (section == 0)
                {
                    section = -1;
                }
                else
                {
                    break;
                }

                continue;
            }

            var parts = line.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length == 0)
            {
                continue;
            }

            if (section == 0 && parts.Length >= 8)
            {
                schedules.Add(
                    new CustomArenaStartTimeEntry
                    {
                        ArenaIndex = GameDataTextTableLoader.ParseInt(parts[0]),
                        Year = GameDataTextTableLoader.ParseIntOrStar(parts[1]),
                        Month = GameDataTextTableLoader.ParseIntOrStar(parts[2]),
                        Day = GameDataTextTableLoader.ParseIntOrStar(parts[3]),
                        DayOfWeek = GameDataTextTableLoader.ParseIntOrStar(parts[4]),
                        Hour = GameDataTextTableLoader.ParseIntOrStar(parts[5]),
                        Minute = GameDataTextTableLoader.ParseIntOrStar(parts[6]),
                        Second = GameDataTextTableLoader.ParseIntOrStar(parts[7]),
                    });
                continue;
            }

            if (section == 1 && parts.Length >= 26)
            {
                var require = new int[7];
                for (var i = 0; i < 7; i++)
                {
                    require[i] = GameDataTextTableLoader.ParseIntOrStar(parts[19 + i]) is var v and >= 0 ? v : 0;
                }

                rules.Add(
                    new CustomArenaRuleEntry
                    {
                        Index = GameDataTextTableLoader.ParseInt(parts[0]),
                        AlarmTime = GameDataTextTableLoader.ParseInt(parts[2]),
                        StandTime = GameDataTextTableLoader.ParseInt(parts[3]),
                        EventTime = GameDataTextTableLoader.ParseInt(parts[4]),
                        CloseTime = GameDataTextTableLoader.ParseInt(parts[5]),
                        StartGate = GameDataTextTableLoader.ParseInt(parts[6]),
                        MinLevel = GameDataTextTableLoader.ParseIntOrStar(parts[11]),
                        MaxLevel = GameDataTextTableLoader.ParseIntOrStar(parts[12]),
                        MinReset = GameDataTextTableLoader.ParseIntOrStar(parts[15]),
                        MaxReset = GameDataTextTableLoader.ParseIntOrStar(parts[16]),
                        RequireClass = require,
                    });
            }
        }

        return new LoadResult(schedules, rules);
    }

    /// <summary>Rules-only loader kept for tests.</summary>
    public static IReadOnlyList<CustomArenaRuleEntry> LoadRulesFromFile(string path) =>
        LoadFromFile(path).Rules;
}
