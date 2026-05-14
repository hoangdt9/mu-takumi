namespace Takumi.Server.Persistence;

/// <summary>Merge DB snapshot into an in-memory roster keyed by normalised name (M4b).</summary>
public static class CharacterRosterMerge
{
    /// <summary>Overlay world + level/class from <paramref name="dbRows"/> onto entries with the same name (ASCII trim).</summary>
    public static void ApplyDbOverlay<T>(
        IReadOnlyList<T> roster,
        IReadOnlyList<CharacterRosterRow> dbRows,
        Func<T, string> getName,
        Action<T, CharacterRosterRow> applyFields)
    {
        if (dbRows.Count == 0)
        {
            return;
        }

        var byName = new Dictionary<string, CharacterRosterRow>(StringComparer.Ordinal);
        foreach (var d in dbRows)
        {
            var k = NormaliseName(d.Name);
            if (k.Length == 0)
            {
                continue;
            }

            byName[k] = d;
        }

        foreach (var e in roster)
        {
            var k = NormaliseName(getName(e));
            if (k.Length == 0 || !byName.TryGetValue(k, out var d))
            {
                continue;
            }

            applyFields(e, d);
        }
    }

    public static string NormaliseName(string name) => name.TrimEnd('\0', ' ').Trim();
}
