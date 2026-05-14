namespace Takumi.Server.Game;

/// <summary>Background loop: while logged in, flush roster when dirty flag is raised (walk / instant move).</summary>
public static class RosterPeriodicSaveRunner
{
    public static async Task RunAsync(
        Func<bool> isLoggedIn,
        Func<int> getDirty,
        Action clearDirty,
        Action flush,
        TimeSpan interval,
        CancellationToken ct)
    {
        while (!ct.IsCancellationRequested)
        {
            try
            {
                await Task.Delay(interval, ct).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (ct.IsCancellationRequested)
            {
                break;
            }

            if (!isLoggedIn())
            {
                continue;
            }

            if (getDirty() == 0)
            {
                continue;
            }

            clearDirty();
            try
            {
                flush();
            }
            catch (Exception ex)
            {
                Console.WriteLine("[roster] periodic save failed: {0}", ex.Message);
            }
        }
    }
}
