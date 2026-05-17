using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CustomArenaScheduleFsmTests
{
    [Fact]
    public void IsEnterEnabled_honors_schedule_fsm_when_not_skipped()
    {
        var prev = Environment.GetEnvironmentVariable("TAKUMI_CUSTOM_ARENA_SKIP_SCHEDULE");
        Environment.SetEnvironmentVariable("TAKUMI_CUSTOM_ARENA_SKIP_SCHEDULE", "0");
        try
        {
            CustomArenaCatalog.LoadForTests(
            [
                new CustomArenaRuleEntry
                {
                    Index = 0,
                    StartGate = 900,
                    AlarmTime = 30,
                    StandTime = 5,
                    EventTime = 10,
                    CloseTime = 5,
                },
            ],
            [
                new CustomArenaStartTimeEntry
                {
                    ArenaIndex = 0,
                    Year = -1,
                    Month = -1,
                    Day = -1,
                    DayOfWeek = -1,
                    Hour = 12,
                    Minute = 0,
                    Second = -1,
                },
            ]);

            CustomArenaScheduleFsm.SetRuntimeForTests(0, CustomArenaScheduleFsm.ArenaState.Empty, 600, enterEnabled: false);
            Assert.False(CustomArenaScheduleFsm.IsEnterEnabled(0));

            CustomArenaScheduleFsm.SetRuntimeForTests(0, CustomArenaScheduleFsm.ArenaState.Empty, 120, enterEnabled: true);
            Assert.True(CustomArenaScheduleFsm.IsEnterEnabled(0));
        }
        finally
        {
            Environment.SetEnvironmentVariable("TAKUMI_CUSTOM_ARENA_SKIP_SCHEDULE", prev);
        }
    }

    [Fact]
    public void EventScheduleCalculator_finds_next_minute_slot()
    {
        var slots = new[]
        {
            new CustomArenaStartTimeEntry
            {
                ArenaIndex = 0,
                Year = -1,
                Month = -1,
                Day = -1,
                DayOfWeek = -1,
                Hour = 15,
                Minute = 30,
                Second = -1,
            },
        };

        var probe = new DateTime(2026, 5, 17, 15, 28, 0);
        var next = EventScheduleCalculator.GetNextOccurrence(slots, probe);
        Assert.NotNull(next);
        Assert.Equal(15, next!.Value.Hour);
        Assert.Equal(30, next.Value.Minute);
    }
}
