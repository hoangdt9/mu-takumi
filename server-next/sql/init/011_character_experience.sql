-- M7: cumulative experience + level progression (parity client ReceiveDieExp / join F3 03 offsets 8+16).
-- Apply: ./scripts/db/apply-sql.sh "$TAKUMI_PG_CONNECTION_STRING"

ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS experience BIGINT NOT NULL DEFAULT 0;

ALTER TABLE character_domain ADD COLUMN IF NOT EXISTS experience BIGINT NOT NULL DEFAULT 0;

COMMENT ON COLUMN character_roster.experience IS 'M7: cumulative EXP (client CharacterAttribute->Experience)';
