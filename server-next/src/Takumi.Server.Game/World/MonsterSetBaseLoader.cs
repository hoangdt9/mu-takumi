using System.Globalization;

namespace Takumi.Server.Game.World;

/// <summary>Loads <c>MonsterSetBase.txt</c> (parity <c>CMonsterSetBase::Load</c>).</summary>
public static class MonsterSetBaseLoader
{
    public static IReadOnlyList<MonsterSetBaseEntry> LoadFromFile(string path)
    {
        if (!File.Exists(path))
        {
            throw new FileNotFoundException("MonsterSetBase file not found.", path);
        }

        var lines = File.ReadAllLines(path);
        var list = new List<MonsterSetBaseEntry>(512);
        var section = -1;
        var i = 0;

        while (i < lines.Length)
        {
            var line = StripComment(lines[i++]).Trim();
            if (line.Length == 0)
            {
                continue;
            }

            if (int.TryParse(line, NumberStyles.Integer, CultureInfo.InvariantCulture, out var sec))
            {
                section = sec;
                continue;
            }

            if (string.Equals(line, "end", StringComparison.OrdinalIgnoreCase))
            {
                section = -1;
                continue;
            }

            if (section < 0)
            {
                continue;
            }

            var parts = line.Split((char[]?)null, StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length < 6)
            {
                continue;
            }

            var monsterClass = ParseInt(parts[0]);
            var map = (byte)ParseInt(parts[1]);
            var dis = ParseInt(parts[2]);
            var x = ParseInt(parts[3]);
            var y = ParseInt(parts[4]);
            var tx = x;
            var ty = y;
            var idx = 5;

            if (section is 1 or 3)
            {
                if (parts.Length < 8)
                {
                    continue;
                }

                tx = ParseInt(parts[5]);
                ty = ParseInt(parts[6]);
                idx = 7;
            }
            else if (section == 2)
            {
                x = x - 3 + Random.Shared.Next(7);
                y = y - 3 + Random.Shared.Next(7);
            }

            var dir = (byte)ParseInt(parts[idx]);
            idx++;

            var value = 0;
            var repeat = 1;
            if (section is 1 or 3)
            {
                if (idx >= parts.Length)
                {
                    continue;
                }

                repeat = ParseInt(parts[idx]);
                idx++;
                if (section == 3 && idx < parts.Length)
                {
                    value = ParseInt(parts[idx]);
                }
            }

            if (dir == 255)
            {
                dir = (byte)Random.Shared.Next(8);
            }

            for (var n = 0; n < repeat; n++)
            {
                list.Add(
                    new MonsterSetBaseEntry
                    {
                        SpawnType = section,
                        MonsterClass = monsterClass,
                        Map = map,
                        Dis = dis,
                        X = x,
                        Y = y,
                        Tx = tx,
                        Ty = ty,
                        Dir = dir,
                        Value = value,
                    });
            }
        }

        return list;
    }

    static string StripComment(string line)
    {
        var hash = line.IndexOf("//", StringComparison.Ordinal);
        return hash >= 0 ? line[..hash] : line;
    }

    static int ParseInt(string s) => int.Parse(s, CultureInfo.InvariantCulture);
}
