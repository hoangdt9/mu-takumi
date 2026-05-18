using Takumi.Server.Persistence;

namespace Takumi.Server.Game;

/// <summary>Logs active roster SSOT mode once per process (M4b).</summary>
public static class CharacterRosterSsoT
{
    static int s_logged;

    public static void LogStartupOnce()
    {
        if (Interlocked.Exchange(ref s_logged, 1) != 0)
        {
            return;
        }

        if (!CharacterRosterBootstrap.IsDbSyncEnabled())
        {
            Console.WriteLine("[roster-ssot] mode=json-only (TAKUMI_ROSTER_DB_SYNC off)");
            return;
        }

        if (CharacterRosterBootstrap.IsDbPrimaryEnabled())
        {
            var jsonExport = CharacterRosterBootstrap.ShouldSkipJsonExportOnSave()
                ? "off"
                : "on (TAKUMI_ROSTER_JSON_EXPORT)";
            Console.WriteLine(
                "[roster-ssot] mode=postgres-primary load=character_roster→character_domain→json-fallback save=db-upsert json-export={0} domain-sync={1}",
                jsonExport,
                CharacterDomainMirrorWriter.IsEnabled());
            return;
        }

        Console.WriteLine(
            "[roster-ssot] mode=json-primary merge=postgres-overlay (set TAKUMI_ROSTER_DB_PRIMARY=1 for DB-first)");
    }
}
