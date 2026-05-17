namespace Takumi.Server.Game.World;

/// <summary>Custom arena event FSM (parity <c>CCustomArena::MainProc</c> / <c>EnterEnabled</c>).</summary>
public static class CustomArenaScheduleFsm
{
    public enum ArenaState
    {
        Blank = 0,
        Empty = 1,
        Stand = 2,
        Start = 3,
        Clean = 4,
    }

    sealed class ArenaRuntime
    {
        public CustomArenaRuleEntry Rule = null!;
        public IReadOnlyList<CustomArenaStartTimeEntry> Schedules = Array.Empty<CustomArenaStartTimeEntry>();
        public ArenaState State = ArenaState.Blank;
        public int RemainTime;
        public DateTime TargetUtc;
        public bool EnterEnabled;
    }

    static readonly object TickLock = new();
    static readonly Dictionary<int, ArenaRuntime> Runtimes = new();
    static bool _initialized;

    public static void EnsureInitialized()
    {
        CustomArenaCatalog.EnsureInitialized();
        if (_initialized)
        {
            return;
        }

        lock (TickLock)
        {
            if (_initialized)
            {
                return;
            }

            RebuildRuntimes();
            _initialized = true;
        }
    }

    public static void RebuildRuntimes()
    {
        Runtimes.Clear();
        foreach (var rule in CustomArenaCatalog.GetAllRules())
        {
            var rt = new ArenaRuntime
            {
                Rule = rule,
                Schedules = CustomArenaCatalog.GetSchedules(rule.Index),
            };
            if (rt.Schedules.Count == 0)
            {
                rt.State = ArenaState.Blank;
                rt.EnterEnabled = false;
            }
            else
            {
                SetStateEmpty(rt);
            }

            Runtimes[rule.Index] = rt;
        }
    }

    public static bool IsEnterEnabled(int arenaIndex)
    {
        if (CustomArenaCatalog.SkipScheduleCheck())
        {
            return true;
        }

        EnsureInitialized();
        lock (TickLock)
        {
            return Runtimes.TryGetValue(arenaIndex, out var rt) && rt.EnterEnabled;
        }
    }

    public static void Tick()
    {
        if (CustomArenaCatalog.SkipScheduleCheck())
        {
            return;
        }

        EnsureInitialized();
        lock (TickLock)
        {
            var now = DateTime.Now;
            foreach (var rt in Runtimes.Values)
            {
                TickArena(rt, now);
            }
        }
    }

    static void TickArena(ArenaRuntime rt, DateTime now)
    {
        if (rt.State == ArenaState.Blank)
        {
            return;
        }

        rt.RemainTime = (int)Math.Max(0, (rt.TargetUtc - now).TotalSeconds);

        switch (rt.State)
        {
            case ArenaState.Empty:
                ProcEmpty(rt);
                break;
            case ArenaState.Stand:
                if (rt.RemainTime <= 0)
                {
                    SetStateStart(rt, now);
                }

                break;
            case ArenaState.Start:
                if (rt.RemainTime <= 0)
                {
                    SetStateClean(rt, now);
                }

                break;
            case ArenaState.Clean:
                if (rt.RemainTime <= 0)
                {
                    SetStateEmpty(rt);
                }

                break;
        }
    }

    static void ProcEmpty(ArenaRuntime rt)
    {
        var alarmSeconds = Math.Max(0, rt.Rule.AlarmTime) * 60;
        rt.EnterEnabled = rt.RemainTime > 0 && rt.RemainTime <= alarmSeconds;
        if (rt.RemainTime <= 0)
        {
            SetStateStand(rt, DateTime.Now);
        }
    }

    static void SetStateEmpty(ArenaRuntime rt)
    {
        rt.State = ArenaState.Empty;
        rt.EnterEnabled = false;
        var next = EventScheduleCalculator.GetNextOccurrence(rt.Schedules, DateTime.Now);
        if (next is null)
        {
            rt.State = ArenaState.Blank;
            return;
        }

        rt.TargetUtc = next.Value;
        rt.RemainTime = (int)Math.Max(0, (rt.TargetUtc - DateTime.Now).TotalSeconds);
    }

    static void SetStateStand(ArenaRuntime rt, DateTime now)
    {
        rt.State = ArenaState.Stand;
        rt.EnterEnabled = false;
        rt.RemainTime = Math.Max(0, rt.Rule.StandTime) * 60;
        rt.TargetUtc = now.AddSeconds(rt.RemainTime);
    }

    static void SetStateStart(ArenaRuntime rt, DateTime now)
    {
        rt.State = ArenaState.Start;
        rt.EnterEnabled = false;
        rt.RemainTime = Math.Max(0, rt.Rule.EventTime) * 60;
        rt.TargetUtc = now.AddSeconds(rt.RemainTime);
    }

    static void SetStateClean(ArenaRuntime rt, DateTime now)
    {
        rt.State = ArenaState.Clean;
        rt.EnterEnabled = false;
        rt.RemainTime = Math.Max(0, rt.Rule.CloseTime) * 60;
        rt.TargetUtc = now.AddSeconds(rt.RemainTime);
    }

    internal static void SetRuntimeForTests(int arenaIndex, ArenaState state, int remainSeconds, bool enterEnabled)
    {
        EnsureInitialized();
        lock (TickLock)
        {
            if (!Runtimes.TryGetValue(arenaIndex, out var rt))
            {
                return;
            }

            rt.State = state;
            rt.RemainTime = remainSeconds;
            rt.EnterEnabled = enterEnabled;
            rt.TargetUtc = DateTime.Now.AddSeconds(remainSeconds);
        }
    }
}
