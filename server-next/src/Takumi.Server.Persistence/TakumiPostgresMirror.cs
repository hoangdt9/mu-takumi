namespace Takumi.Server.Persistence;

/// <summary>Optional Postgres readers when <c>TAKUMI_ROSTER_DB_SYNC</c> is enabled (roster + inventory).</summary>
public static class TakumiPostgresMirror
{
    public static PostgresCharacterRosterRepository? CharacterRoster { get; private set; }

    public static PostgresInventorySlotRepository? InventorySlots { get; private set; }

    /// <summary>Optional <c>session_ticket</c> rows for login→game handoff when <c>TAKUMI_SESSION_HANDOFF_DB</c> is on.</summary>
    public static PostgresSessionHandoffRepository? SessionHandoff { get; private set; }

    /// <summary>Reads <c>TAKUMI_ROSTER_DB_SYNC</c> and connection env; no-op when disabled or misconfigured.</summary>
    public static void InitIfEnabled()
    {
        CharacterRoster = null;
        InventorySlots = null;
        var sync = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC"), "1", StringComparison.OrdinalIgnoreCase)
                   || string.Equals(Environment.GetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC"), "true", StringComparison.OrdinalIgnoreCase);
        if (!sync)
        {
            return;
        }

        var cs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv();
        if (string.IsNullOrEmpty(cs))
        {
            Console.Error.WriteLine(
                "[postgres-mirror] TAKUMI_ROSTER_DB_SYNC is set but no connection string: set TAKUMI_PG_CONNECTION_STRING or TAKUMI_PG_HOST (+ user/password/database).");
            return;
        }

        try
        {
            CharacterRoster = new PostgresCharacterRosterRepository(cs);
            InventorySlots = new PostgresInventorySlotRepository(cs);
            Console.Error.WriteLine("[postgres-mirror] roster + inventory_slot readers enabled (JSON roster still authoritative for list).");
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine("[postgres-mirror] init failed (JSON-only): {0}", ex.Message);
            CharacterRoster = null;
            InventorySlots = null;
        }
    }

    /// <summary>
    /// Enables <see cref="SessionHandoff"/> when <c>TAKUMI_SESSION_HANDOFF_DB=1</c> (or <c>true</c>) and a Postgres connection string is set.
    /// Independent of <c>TAKUMI_ROSTER_DB_SYNC</c>; requires <c>sql/init/003_session_ticket.sql</c> applied.
    /// </summary>
    public static void InitSessionHandoffIfEnabled()
    {
        SessionHandoff = null;
        var on = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_SESSION_HANDOFF_DB"), "1", StringComparison.OrdinalIgnoreCase)
                 || string.Equals(Environment.GetEnvironmentVariable("TAKUMI_SESSION_HANDOFF_DB"), "true", StringComparison.OrdinalIgnoreCase);
        if (!on)
        {
            return;
        }

        var cs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv();
        if (string.IsNullOrEmpty(cs))
        {
            Console.Error.WriteLine(
                "[session-handoff-db] TAKUMI_SESSION_HANDOFF_DB is set but no connection string: set TAKUMI_PG_CONNECTION_STRING or TAKUMI_PG_HOST (+ user/password/database).");
            return;
        }

        try
        {
            SessionHandoff = new PostgresSessionHandoffRepository(cs);
            Console.Error.WriteLine("[session-handoff-db] session_ticket persistence enabled (login host writes; game host may require consume).");
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine("[session-handoff-db] init failed: {0}", ex.Message);
            SessionHandoff = null;
        }
    }
}
