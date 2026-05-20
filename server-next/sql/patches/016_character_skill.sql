-- Learned skills per character (C1 F3 11 / GCSkillListSend parity).
-- Apply: ./scripts/db/apply-sql.sh "$TAKUMI_PG_CONNECTION_STRING" sql/patches/016_character_skill.sql

CREATE TABLE IF NOT EXISTS character_skill (
    account_login   TEXT        NOT NULL,
    character_name  TEXT        NOT NULL,
    skill_slot      SMALLINT    NOT NULL,
    skill_type      SMALLINT    NOT NULL,
    skill_level     SMALLINT    NOT NULL DEFAULT 0,
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (account_login, character_name, skill_slot)
);

CREATE INDEX IF NOT EXISTS idx_character_skill_char
    ON character_skill (account_login, character_name);

COMMENT ON TABLE character_skill IS 'Client Skill[] slots: skill_slot = array index, skill_type = AT_SKILL_* id';
