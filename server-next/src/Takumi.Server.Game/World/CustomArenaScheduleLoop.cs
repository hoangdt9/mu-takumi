namespace Takumi.Server.Game.World;

/// <summary>1 Hz tick for <see cref="CustomArenaScheduleFsm"/> when schedule checks are enabled.</summary>
public static class CustomArenaScheduleLoop
{
    static Task? _loopTask;

    public static void Start(CancellationToken appCancellationToken)
    {
        if (_loopTask is not null || CustomArenaCatalog.SkipScheduleCheck())
        {
            return;
        }

        CustomArenaScheduleFsm.EnsureInitialized();
        _loopTask = Task.Run(() => RunAsync(appCancellationToken), appCancellationToken);
        Console.WriteLine("[m8] CustomArenaScheduleLoop started (TAKUMI_CUSTOM_ARENA_SKIP_SCHEDULE=0)");
    }

    static async Task RunAsync(CancellationToken ct)
    {
        using var timer = new PeriodicTimer(TimeSpan.FromSeconds(1));
        try
        {
            while (await timer.WaitForNextTickAsync(ct).ConfigureAwait(false))
            {
                CustomArenaScheduleFsm.Tick();
            }
        }
        catch (OperationCanceledException)
        {
            // shutdown
        }
    }
}
