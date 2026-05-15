-- M8: static monster spawns (ETL from MonsterSetBase*.txt).
-- Apply on existing DBs: ./scripts/apply-sql.sh 'postgresql://...'

CREATE TABLE IF NOT EXISTS monster_spawn (
    id              SERIAL PRIMARY KEY,
    spawn_type      SMALLINT    NOT NULL,
    monster_class   INTEGER     NOT NULL,
    map_id          SMALLINT    NOT NULL,
    dis             INTEGER     NOT NULL DEFAULT 0,
    pos_x           SMALLINT    NOT NULL,
    pos_y           SMALLINT    NOT NULL,
    range_tx        SMALLINT    NOT NULL DEFAULT 0,
    range_ty        SMALLINT    NOT NULL DEFAULT 0,
    dir             SMALLINT    NOT NULL DEFAULT 0,
    spawn_value     INTEGER     NOT NULL DEFAULT 0,
    source_file     TEXT,
    loaded_at       TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_monster_spawn_map ON monster_spawn (map_id);
