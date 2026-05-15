using System.Globalization;
using Takumi.Server.Game.Networking;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>M7d: HP regen tick + auto-revive after death.</summary>
public static class PlayerVitalsLoop
{
    static Task? _loopTask;

    public static void Start(CancellationToken appCt)
    {
        if (_loopTask is not null)
        {
            return;
        }

        _loopTask = Task.Run(() => RunAsync(appCt), appCt);
    }

    static async Task RunAsync(CancellationToken ct)
    {
        var interval = TimeSpan.FromMilliseconds(ParseIntEnv("TAKUMI_PLAYER_VITALS_INTERVAL_MS", 2000, 500, 30_000));
        while (!ct.IsCancellationRequested)
        {
            try
            {
                foreach (var session in MonsterViewerRegistry.GetAllSessions())
                {
                    if (session.PlayerObjectKey == 0)
                    {
                        continue;
                    }

                    if (PlayerVitalsState.TryGetReviveDue(session.SessionId, out var reviveDue) && reviveDue)
                    {
                        await TryReviveSessionAsync(session, ct).ConfigureAwait(false);
                        continue;
                    }

                    if (PlayerVitalsState.IsDead(session.SessionId) || session.MaxHp <= 0)
                    {
                        continue;
                    }

                    if (session.CurrentHp > 0 && session.CurrentHp < session.MaxHp)
                    {
                        await TryRegenSessionAsync(session, ct).ConfigureAwait(false);
                    }
                }
            }
            catch (OperationCanceledException) when (ct.IsCancellationRequested)
            {
                break;
            }
            catch (Exception ex)
            {
                Console.WriteLine("[m7d] vitals loop error: {0}", ex.Message);
            }

            await Task.Delay(interval, ct).ConfigureAwait(false);
        }
    }

    static async Task TryRegenSessionAsync(MonsterViewerSession session, CancellationToken ct)
    {
        var pct = ParseIntEnv("TAKUMI_PLAYER_HP_REGEN_PCT", 2, 1, 50);
        var add = Math.Max(1, session.MaxHp * pct / 100);
        var next = Math.Min(session.MaxHp, session.CurrentHp + add);
        if (next == session.CurrentHp)
        {
            return;
        }

        session.CurrentHp = next;
        session.OnVitalsChanged?.Invoke(session.CurrentHp, session.MaxHp);
        RosterVitalsCombat.ScheduleVitalsMirror(
            session.AccountLogin,
            session.CharacterName,
            session.CurrentHp,
            session.MaxHp,
            session.CurrentMp,
            session.MaxMp);

        var pkt = LifeManaWire602.BuildLife(LifeManaWire602.TypeCurrent, (ushort)Math.Clamp(next, 0, ushort.MaxValue));
        await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, pkt, ct).ConfigureAwait(false);
    }

    static async Task TryReviveSessionAsync(MonsterViewerSession session, CancellationToken ct)
    {
        PlayerVitalsState.TryClearDead(session.SessionId);
        var maxHp = Math.Max(1, session.MaxHp);
        var maxMp = Math.Max(1, session.MaxMp);
        session.CurrentHp = maxHp;
        session.CurrentMp = maxMp;
        session.OnVitalsChanged?.Invoke(session.CurrentHp, maxHp);
        RosterVitalsCombat.ScheduleVitalsMirror(
            session.AccountLogin,
            session.CharacterName,
            session.CurrentHp,
            maxHp,
            session.CurrentMp,
            maxMp);

        var regenPkt = CharacterRegenWire602.Build(
            session.MapId,
            session.X,
            session.Y,
            dir: 0,
            life: (ushort)Math.Clamp(session.CurrentHp, 0, ushort.MaxValue),
            mana: (ushort)Math.Clamp(session.CurrentMp, 0, ushort.MaxValue));
        await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, regenPkt, ct).ConfigureAwait(false);
        await GamePortOutboundWire.WriteAsync(
                session.Connection,
                session.Protect,
                LifeManaWire602.BuildLife(LifeManaWire602.TypeCurrent, (ushort)Math.Min(session.CurrentHp, ushort.MaxValue)),
                ct)
            .ConfigureAwait(false);
        await GamePortOutboundWire.WriteAsync(
                session.Connection,
                session.Protect,
                LifeManaWire602.BuildMana(LifeManaWire602.TypeCurrent, (ushort)Math.Min(session.CurrentMp, ushort.MaxValue)),
                ct)
            .ConfigureAwait(false);

        Console.WriteLine("[m7d] player revived key={0} hp={1}", session.PlayerObjectKey, session.CurrentHp);
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
