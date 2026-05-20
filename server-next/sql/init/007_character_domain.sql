-- M4b/M7: domain mirror of roster world + vitals (one-way sync from character_roster writes).
-- Apply on existing DBs: ./scripts/db/apply-sql.sh 'postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime'

CREATE TABLE IF NOT EXISTS character_domain (
    account_login   TEXT        NOT NULL,
    character_name  TEXT        NOT NULL,
    server_class    SMALLINT    NOT NULL,
    level           INTEGER     NOT NULL DEFAULT 1,
    map_id          SMALLINT    NOT NULL DEFAULT 0,
    pos_x           SMALLINT    NOT NULL DEFAULT 0,
    pos_y           SMALLINT    NOT NULL DEFAULT 0,
    angle           SMALLINT    NOT NULL DEFAULT 0,
    current_hp      INTEGER     NOT NULL DEFAULT 0,
    max_hp          INTEGER     NOT NULL DEFAULT 0,
    current_mp      INTEGER     NOT NULL DEFAULT 0,
    max_mp          INTEGER     NOT NULL DEFAULT 0,
    zen             BIGINT      NOT NULL DEFAULT 0,
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (account_login, character_name)
);

CREATE INDEX IF NOT EXISTS idx_character_domain_account ON character_domain (account_login);

-- Optional ETL source (populate via external scripts; importer no-ops when empty).
CREATE TABLE IF NOT EXISTS character_staging (
    account_login   TEXT        NOT NULL,
    character_name  TEXT        NOT NULL,
    server_class    SMALLINT    NOT NULL DEFAULT 0,
    level           INTEGER     NOT NULL DEFAULT 1,
    map_id          SMALLINT    NOT NULL DEFAULT 0,
    pos_x           SMALLINT    NOT NULL DEFAULT 0,
    pos_y           SMALLINT    NOT NULL DEFAULT 0,
    angle           SMALLINT    NOT NULL DEFAULT 0,
    current_hp      INTEGER     NOT NULL DEFAULT 0,
    max_hp          INTEGER     NOT NULL DEFAULT 0,
    current_mp      INTEGER     NOT NULL DEFAULT 0,
    max_mp          INTEGER     NOT NULL DEFAULT 0,
    zen             BIGINT      NOT NULL DEFAULT 0,
    PRIMARY KEY (account_login, character_name)
);
