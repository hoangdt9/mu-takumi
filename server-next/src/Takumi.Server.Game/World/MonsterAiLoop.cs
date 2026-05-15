using System.Globalization;
using Takumi.Server.Game.Networking;

namespace Takumi.Server.Game.World;

public enum MonsterAiEventKind
{
    Walk,
    Attack,
    SkillAttack,
    Regen,
}

public readonly record struct MonsterAiEvent(
    MonsterAiEventKind Kind,
    int ObjectKey,
    byte MapId,
    byte X,
    byte Y,
    byte Dir,
    int TargetObjectKey,
    Guid TargetSessionId = default);

/// <summary>Background tick for monster wander + idle attack (parity <c>gObjMonsterProcess</c> / <c>SendMonsterMoveMsg</c>).</summary>
public static class MonsterAiLoop
{
    static Task? _loopTask;

    public static void Start(CancellationToken appCancellationToken)
    {
        if (_loopTask is not null)
        {
            return;
        }

        MapMonsterWorld.EnsureInitialized();
        _loopTask = Task.Run(() => RunAsync(appCancellationToken), appCancellationToken);
        MonsterViewportPeriodicLoop.Start(appCancellationToken);
        if (!MonsterViewerRegistry.IsAiEnabled)
        {
            Console.WriteLine("[m9-ai] wander/attack disabled (TAKUMI_MONSTER_AI_ENABLED=0); viewport periodic may still run");
        }
        else
        {
            Console.WriteLine(
                "[m9-ai] monster AI loop started intervalMs={0}",
                ParseIntEnv("TAKUMI_MONSTER_AI_INTERVAL_MS", 500, 100, 5000));
        }
    }

    static async Task RunAsync(CancellationToken ct)
    {
        var interval = TimeSpan.FromMilliseconds(
            ParseIntEnv("TAKUMI_MONSTER_AI_INTERVAL_MS", 500, 100, 5000));
        var rng = Random.Shared;
        while (!ct.IsCancellationRequested)
        {
            try
            {
                if (!MonsterViewerRegistry.IsAiEnabled)
                {
                    await Task.Delay(interval, ct).ConfigureAwait(false);
                    continue;
                }

                var events = MapMonsterWorld.ProcessAiTick(rng);
                foreach (var ev in events)
                {
                    switch (ev.Kind)
                    {
                        case MonsterAiEventKind.Walk:
                            await MonsterViewerRegistry.BroadcastWalkAsync(
                                    ev.ObjectKey,
                                    ev.MapId,
                                    ev.X,
                                    ev.Y,
                                    ct)
                                .ConfigureAwait(false);
                            break;
                        case MonsterAiEventKind.Attack:
                        case MonsterAiEventKind.SkillAttack:
                            await MonsterViewerRegistry.BroadcastMonsterActionAsync(
                                    ev.ObjectKey,
                                    ev.MapId,
                                    ev.X,
                                    ev.Y,
                                    ev.Dir,
                                    action: 120,
                                    ev.TargetObjectKey,
                                    ct)
                                .ConfigureAwait(false);
                            if (ev.TargetSessionId != Guid.Empty)
                            {
                                var skillMult = ev.Kind == MonsterAiEventKind.SkillAttack
                                    ? ParseIntEnv("TAKUMI_MONSTER_SKILL_DAMAGE_PCT", 150, 50, 500)
                                    : 100;
                                await MonsterViewerRegistry.ApplyMonsterHitToPlayerAsync(
                                        ev.TargetSessionId,
                                        ev.ObjectKey,
                                        ev.MapId,
                                        ev.X,
                                        ev.Y,
                                        skillMult,
                                        ct)
                                    .ConfigureAwait(false);
                            }

                            break;
                        case MonsterAiEventKind.Regen:
                            if (MapMonsterWorld.TryGetMonster(ev.ObjectKey, out var regenMob) && regenMob is not null)
                            {
                                await MonsterViewportBroadcast.RegenMonsterAsync(regenMob, ct).ConfigureAwait(false);
                            }

                            break;
                    }
                }
            }
            catch (OperationCanceledException) when (ct.IsCancellationRequested)
            {
                break;
            }
            catch (Exception ex)
            {
                Console.WriteLine("[m9-ai] tick error: {0}", ex.Message);
            }

            try
            {
                await Task.Delay(interval, ct).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (ct.IsCancellationRequested)
            {
                break;
            }
        }
    }

    static int ParseIntEnv(string key, int defaultValue, int min, int max)
    {
        var raw = Environment.GetEnvironmentVariable(key);
        if (string.IsNullOrWhiteSpace(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var v))
        {
            return defaultValue;
        }

        return Math.Clamp(v, min, max);
    }
}
