namespace Takumi.Server.Persistence;

/// <summary>Optional Postgres readers when <c>TAKUMI_ROSTER_DB_SYNC</c> is enabled (roster + inventory).</summary>
public static class TakumiPostgresMirror
{
    public static PostgresCharacterRosterRepository? CharacterRoster { get; private set; }

    public static PostgresCharacterDomainRepository? CharacterDomain { get; private set; }

    public static PostgresInventorySlotRepository? InventorySlots { get; private set; }

    /// <summary>Optional <c>session_ticket</c> rows for login→game handoff when <c>TAKUMI_SESSION_HANDOFF_DB</c> is on.</summary>
    public static PostgresSessionHandoffRepository? SessionHandoff { get; private set; }

    /// <summary>Optional <c>monster_spawn</c> reader when <c>TAKUMI_MONSTER_SPAWN_DB</c> is on (M8).</summary>
    public static PostgresMonsterSpawnRepository? MonsterSpawn { get; private set; }

    public static PostgresMapGateRepository? MapGate { get; private set; }

    public static PostgresNpcShopRepository? NpcShop { get; private set; }

    public static PostgresCustomWorldConfigRepository? CustomWorld { get; private set; }

    /// <summary>Reads <c>TAKUMI_ROSTER_DB_SYNC</c> and connection env; no-op when disabled or misconfigured.</summary>
    public static void InitIfEnabled()
    {
        CharacterRoster = null;
        CharacterDomain = null;
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
            if (CharacterDomainMirrorWriter.IsEnabled())
            {
                CharacterDomain = new PostgresCharacterDomainRepository(cs);
            }

            InventorySlots = new PostgresInventorySlotRepository(cs);
            Console.Error.WriteLine(
                "[postgres-mirror] roster + inventory_slot enabled; character_domain={0}",
                CharacterDomain is not null);
            _ = CharacterLegacyWorldImporter.TryImportAsync();
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine("[postgres-mirror] init failed (JSON-only): {0}", ex.Message);
            CharacterRoster = null;
            CharacterDomain = null;
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

    /// <summary>
    /// Enables <see cref="MonsterSpawn"/> when <c>TAKUMI_MONSTER_SPAWN_DB=1</c> (or <c>true</c>) and a Postgres connection string is set.
    /// Requires <c>sql/init/005_monster_spawn.sql</c> applied; populate via <c>MonsterSpawnDbImporter</c> / <c>scripts/import-monster-spawn.sh</c>.
    /// </summary>
    public static void InitMonsterSpawnIfEnabled()
    {
        MonsterSpawn = null;
        var on = string.Equals(Environment.GetEnvironmentVariable("TAKUMI_MONSTER_SPAWN_DB"), "1", StringComparison.OrdinalIgnoreCase)
                 || string.Equals(Environment.GetEnvironmentVariable("TAKUMI_MONSTER_SPAWN_DB"), "true", StringComparison.OrdinalIgnoreCase);
        if (!on)
        {
            return;
        }

        var cs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv();
        if (string.IsNullOrEmpty(cs))
        {
            Console.Error.WriteLine(
                "[monster-spawn-db] TAKUMI_MONSTER_SPAWN_DB is set but no connection string: set TAKUMI_PG_CONNECTION_STRING or TAKUMI_PG_HOST (+ user/password/database).");
            return;
        }

        try
        {
            MonsterSpawn = new PostgresMonsterSpawnRepository(cs);
            Console.Error.WriteLine("[monster-spawn-db] monster_spawn reader enabled (MapMonsterWorld prefers DB over file).");
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine("[monster-spawn-db] init failed: {0}", ex.Message);
            MonsterSpawn = null;
        }
    }

    /// <summary>
    /// Enables map_gate / npc_shop / custom_world_config readers when <c>TAKUMI_WORLD_STATIC_DB=1</c>
    /// or individual <c>TAKUMI_MAP_GATE_DB</c>, <c>TAKUMI_NPC_SHOP_DB</c>, <c>TAKUMI_CUSTOM_WORLD_DB</c> flags are set.
    /// Requires <c>sql/init/006_map_gate_npc_shop_custom.sql</c> applied.
    /// </summary>
    public static void InitWorldStaticDataIfEnabled()
    {
        MapGate = null;
        NpcShop = null;
        CustomWorld = null;

        var all = EnvOn("TAKUMI_WORLD_STATIC_DB");
        var gateOn = all || EnvOn("TAKUMI_MAP_GATE_DB");
        var shopOn = all || EnvOn("TAKUMI_NPC_SHOP_DB");
        var customOn = all || EnvOn("TAKUMI_CUSTOM_WORLD_DB");
        if (!gateOn && !shopOn && !customOn)
        {
            return;
        }

        var cs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv();
        if (string.IsNullOrEmpty(cs))
        {
            Console.Error.WriteLine(
                "[world-static-db] world static DB is enabled but no connection string: set TAKUMI_PG_CONNECTION_STRING or TAKUMI_PG_HOST.");
            return;
        }

        try
        {
            if (gateOn)
            {
                MapGate = new PostgresMapGateRepository(cs);
            }

            if (shopOn)
            {
                NpcShop = new PostgresNpcShopRepository(cs);
            }

            if (customOn)
            {
                CustomWorld = new PostgresCustomWorldConfigRepository(cs);
            }

            Console.Error.WriteLine(
                "[world-static-db] enabled gate={0} shop={1} custom={2}",
                gateOn,
                shopOn,
                customOn);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine("[world-static-db] init failed: {0}", ex.Message);
            MapGate = null;
            NpcShop = null;
            CustomWorld = null;
        }
    }

    static bool EnvOn(string key) =>
        string.Equals(Environment.GetEnvironmentVariable(key), "1", StringComparison.OrdinalIgnoreCase)
        || string.Equals(Environment.GetEnvironmentVariable(key), "true", StringComparison.OrdinalIgnoreCase);
}
