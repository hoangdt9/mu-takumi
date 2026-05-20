-- M7 (schema prep): vitals + zen alongside world columns on character_roster.
-- Apply on existing DBs: ./scripts/db/apply-sql.sh "postgresql://…"  (runs all sql/init/*.sql in lexical order).
-- Fresh Docker volume: picked up with other init scripts.

ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS current_hp INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS max_hp INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS current_mp INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS max_mp INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS zen BIGINT NOT NULL DEFAULT 0;

COMMENT ON COLUMN character_roster.current_hp IS 'M7: last known HP (wire/UI); 0 = unset / use join stub';
COMMENT ON COLUMN character_roster.zen IS 'M7: zen / gold bank (Season6 naming varies by client build)';
