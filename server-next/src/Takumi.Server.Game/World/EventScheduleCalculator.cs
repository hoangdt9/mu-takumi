namespace Takumi.Server.Game.World;

/// <summary>Next-fire schedule helper (parity <c>CScheduleManager</c> for custom arena).</summary>
public static class EventScheduleCalculator
{
    public static DateTime? GetNextOccurrence(IReadOnlyList<CustomArenaStartTimeEntry> slots, DateTime now)
    {
        if (slots.Count == 0)
        {
            return null;
        }

        DateTime? best = null;
        var horizon = now.AddDays(14);
        for (var probe = new DateTime(now.Year, now.Month, now.Day, now.Hour, now.Minute, 0, now.Kind).AddMinutes(1);
             probe < horizon;
             probe = probe.AddMinutes(1))
        {
            foreach (var slot in slots)
            {
                if (!Matches(slot, probe))
                {
                    continue;
                }

                if (best is null || probe < best.Value)
                {
                    best = probe;
                }

                break;
            }
        }

        return best;
    }

    static bool Matches(CustomArenaStartTimeEntry slot, DateTime t)
    {
        if (slot.Year >= 0 && t.Year != slot.Year)
        {
            return false;
        }

        if (slot.Month >= 0 && t.Month != slot.Month)
        {
            return false;
        }

        if (slot.Day >= 0 && t.Day != slot.Day)
        {
            return false;
        }

        if (slot.DayOfWeek >= 0 && ToLegacyDayOfWeek(t.DayOfWeek) != slot.DayOfWeek)
        {
            return false;
        }

        if (slot.Hour >= 0 && t.Hour != slot.Hour)
        {
            return false;
        }

        if (slot.Minute >= 0 && t.Minute != slot.Minute)
        {
            return false;
        }

        if (slot.Second >= 0 && t.Second != slot.Second)
        {
            return false;
        }

        return true;
    }

    /// <summary>Legacy tables use 1=Mon … 7=Sun.</summary>
    static int ToLegacyDayOfWeek(DayOfWeek dow) =>
        dow == DayOfWeek.Sunday ? 7 : (int)dow;
}
