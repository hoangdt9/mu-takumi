using System.Globalization;
using Takumi.Server.Game.Networking;
using Takumi.Server.Game.World;
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
            session.MaxMp,
            session.CurrentShield,
            session.MaxShield);

        var sdWire = (ushort)Math.Clamp(session.CurrentShield, 0, ushort.MaxValue);
        var pkt = LifeManaWire602.BuildLife(LifeManaWire602.TypeCurrent, (ushort)Math.Clamp(next, 0, ushort.MaxValue), sdWire);
        await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, pkt, ct).ConfigureAwait(false);
    }

    static async Task TryReviveSessionAsync(MonsterViewerSession session, CancellationToken ct)
    {
        PlayerVitalsState.TryClearDead(session.SessionId);
        var maxHp = Math.Max(1, session.MaxHp);
        var maxMp = Math.Max(1, session.MaxMp);
        session.CurrentHp = maxHp;
        session.CurrentMp = maxMp;
        if (session.MaxShield > 0)
        {
            session.CurrentShield = session.MaxShield;
        }

        session.OnVitalsChanged?.Invoke(session.CurrentHp, maxHp);
        session.OnShieldVitalsChanged?.Invoke(session.CurrentShield, session.MaxShield);
        RosterVitalsCombat.ScheduleVitalsMirror(
            session.AccountLogin,
            session.CharacterName,
            session.CurrentHp,
            maxHp,
            session.CurrentMp,
            maxMp,
            session.CurrentShield,
            session.MaxShield);

        var town = MapRespawnCatalog.GetTownRespawn(session.MapId);
        session.MapId = town.Map;
        session.X = town.PositionX;
        session.Y = town.PositionY;
        MonsterViewerRegistry.UpdatePosition(session.SessionId, town.Map, town.PositionX, town.PositionY);
        session.OnRosterPositionChanged?.Invoke(town.Map, town.PositionX, town.PositionY);

        if (session.ViewportTracker is not null)
        {
            session.ViewportTracker.ResetForMap(town.Map, town.PositionX, town.PositionY);
            await MapMonsterScopeSender.TrySendAfterJoinAsync(
                    session.ViewportTracker,
                    session.Connection,
                    session.Protect,
                    town.Map,
                    town.PositionX,
                    town.PositionY,
                    "revive",
                    ct)
                .ConfigureAwait(false);
        }

        // Town respawn: C1 F3 04 (ReceiveRevival) — matches client PRECEIVE_REVIVAL (SubCode + tile XY).
        // Do not use C1 0x1C flag=0 here: client struct has no SubCode and misreads XY as (tileY, angle).
        var hp = (ushort)Math.Min(session.CurrentHp, ushort.MaxValue);
        var mp = (ushort)Math.Min(session.CurrentMp, ushort.MaxValue);
        var sd = (ushort)Math.Clamp(session.CurrentShield, 0, ushort.MaxValue);
        var regen = CharacterRegenWire602.Build(
            town.Map,
            town.PositionX,
            town.PositionY,
            town.Angle,
            hp,
            mp,
            shield: sd,
            viewCurHp: (uint)session.CurrentHp,
            viewCurMp: (uint)session.CurrentMp,
            viewCurSd: sd);
        await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, regen, ct).ConfigureAwait(false);

        var hpMaxWire = (ushort)Math.Min(maxHp, ushort.MaxValue);
        var mpMaxWire = (ushort)Math.Min(maxMp, ushort.MaxValue);
        await GamePortOutboundWire.WriteAsync(
                session.Connection,
                session.Protect,
                LifeManaWire602.BuildLife(LifeManaWire602.TypeMax, hpMaxWire, sd),
                ct)
            .ConfigureAwait(false);
        await GamePortOutboundWire.WriteAsync(
                session.Connection,
                session.Protect,
                LifeManaWire602.BuildLife(LifeManaWire602.TypeCurrent, hp, sd),
                ct)
            .ConfigureAwait(false);
        await GamePortOutboundWire.WriteAsync(
                session.Connection,
                session.Protect,
                LifeManaWire602.BuildMana(LifeManaWire602.TypeMax, mpMaxWire, 0),
                ct)
            .ConfigureAwait(false);
        await GamePortOutboundWire.WriteAsync(
                session.Connection,
                session.Protect,
                LifeManaWire602.BuildMana(LifeManaWire602.TypeCurrent, mp, 0),
                ct)
            .ConfigureAwait(false);

        Console.WriteLine(
            "[m7d] player revived key={0} hp={1}/{2} town map={3} xy=({4},{5})",
            session.PlayerObjectKey,
            session.CurrentHp,
            maxHp,
            town.Map,
            town.PositionX,
            town.PositionY);
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
