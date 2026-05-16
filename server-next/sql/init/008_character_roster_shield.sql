-- M7: SD / shield columns (legacy GCLifeSend shield field) on roster + domain mirror.
-- Apply with ./scripts/apply-sql.sh (runs init scripts in lexical order).

ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS current_shield INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS max_shield INTEGER NOT NULL DEFAULT 0;

ALTER TABLE character_domain ADD COLUMN IF NOT EXISTS current_shield INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_domain ADD COLUMN IF NOT EXISTS max_shield INTEGER NOT NULL DEFAULT 0;

COMMENT ON COLUMN character_roster.current_shield IS 'M7: SD current (0 = unset / full at join seed)';
COMMENT ON COLUMN character_roster.max_shield IS 'M7: SD max; 0 = no SD bar';
