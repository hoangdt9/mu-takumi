-- M7: base stats + level-up points + AG (BP) for join wire and stat allocation.
-- Apply: ./scripts/db/apply-sql.sh 'postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime'

ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS strength INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS dexterity INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS vitality INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS energy INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS leadership INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS level_up_point INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS current_bp INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS max_bp INTEGER NOT NULL DEFAULT 0;

ALTER TABLE character_domain ADD COLUMN IF NOT EXISTS strength INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_domain ADD COLUMN IF NOT EXISTS dexterity INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_domain ADD COLUMN IF NOT EXISTS vitality INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_domain ADD COLUMN IF NOT EXISTS energy INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_domain ADD COLUMN IF NOT EXISTS leadership INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_domain ADD COLUMN IF NOT EXISTS level_up_point INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_domain ADD COLUMN IF NOT EXISTS current_bp INTEGER NOT NULL DEFAULT 0;
ALTER TABLE character_domain ADD COLUMN IF NOT EXISTS max_bp INTEGER NOT NULL DEFAULT 0;

COMMENT ON COLUMN character_roster.strength IS 'M7: 0 = derive from class default at join';
COMMENT ON COLUMN character_roster.level_up_point IS 'M7: unspent stat points (F3 06)';
