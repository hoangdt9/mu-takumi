-- M4b: minimal roster table for LegacyLoginHost JSON ↔ Postgres sync.
-- Apply manually on existing DBs: psql "$TAKUMI_PG_CONNECTION_STRING" -v ON_ERROR_STOP=1 -f sql/init/001_character_roster.sql
-- Fresh Docker volume: mount this folder as /docker-entrypoint-initdb.d (see docker-compose comment).

CREATE TABLE IF NOT EXISTS character_roster (
    account_login   TEXT        NOT NULL,
    character_name  TEXT        NOT NULL,
    server_class    SMALLINT    NOT NULL,
    level           INTEGER     NOT NULL DEFAULT 1,
    map_id          SMALLINT    NOT NULL DEFAULT 0,
    pos_x           SMALLINT    NOT NULL DEFAULT 0,
    pos_y           SMALLINT    NOT NULL DEFAULT 0,
    angle           SMALLINT    NOT NULL DEFAULT 0,
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (account_login, character_name)
);

CREATE INDEX IF NOT EXISTS idx_character_roster_account ON character_roster (account_login);
