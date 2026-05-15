-- M8: map gates (Gate.txt), NPC shops (ShopManager + Shop/*.txt), Custom/*.txt snapshots.
-- Apply: ./scripts/apply-sql.sh 'postgresql://...'

CREATE TABLE IF NOT EXISTS map_gate (
    id              SERIAL PRIMARY KEY,
    gate_index      INTEGER     NOT NULL UNIQUE,
    flag            SMALLINT    NOT NULL,
    map_id          SMALLINT    NOT NULL,
    pos_x           SMALLINT    NOT NULL,
    pos_y           SMALLINT    NOT NULL,
    range_tx        SMALLINT    NOT NULL,
    range_ty        SMALLINT    NOT NULL,
    target_gate     INTEGER     NOT NULL,
    dir             SMALLINT    NOT NULL,
    min_level       INTEGER     NOT NULL DEFAULT -1,
    max_level       INTEGER     NOT NULL DEFAULT -1,
    min_reset       INTEGER     NOT NULL DEFAULT -1,
    max_reset       INTEGER     NOT NULL DEFAULT -1,
    account_level   SMALLINT    NOT NULL DEFAULT 0,
    source_file     TEXT,
    loaded_at       TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_map_gate_map ON map_gate (map_id);

CREATE TABLE IF NOT EXISTS npc_shop (
    shop_index      INTEGER     PRIMARY KEY,
    monster_class   INTEGER     NOT NULL,
    map_id          SMALLINT,
    pos_x           SMALLINT,
    pos_y           SMALLINT,
    comment         TEXT,
    source_file     TEXT,
    loaded_at       TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS npc_shop_item (
    id              SERIAL PRIMARY KEY,
    shop_index      INTEGER     NOT NULL REFERENCES npc_shop (shop_index) ON DELETE CASCADE,
    slot            SMALLINT    NOT NULL,
    item_group      SMALLINT    NOT NULL,
    item_index      SMALLINT    NOT NULL,
    item_level      SMALLINT    NOT NULL DEFAULT 0,
    durability      SMALLINT    NOT NULL DEFAULT 0,
    skill           SMALLINT    NOT NULL DEFAULT 0,
    luck            SMALLINT    NOT NULL DEFAULT 0,
    option          SMALLINT    NOT NULL DEFAULT 0,
    exc_opt         SMALLINT    NOT NULL DEFAULT 0,
    anc             SMALLINT    NOT NULL DEFAULT 0,
    joh             SMALLINT    NOT NULL DEFAULT 0,
    oex             SMALLINT    NOT NULL DEFAULT 0,
    socket1         SMALLINT    NOT NULL DEFAULT 255,
    socket2         SMALLINT    NOT NULL DEFAULT 255,
    socket3         SMALLINT    NOT NULL DEFAULT 255,
    socket4         SMALLINT    NOT NULL DEFAULT 255,
    socket5         SMALLINT    NOT NULL DEFAULT 255,
    item_name       TEXT,
    source_file     TEXT,
    loaded_at       TIMESTAMPTZ NOT NULL DEFAULT now(),
    UNIQUE (shop_index, slot)
);

CREATE INDEX IF NOT EXISTS idx_npc_shop_item_shop ON npc_shop_item (shop_index);

CREATE TABLE IF NOT EXISTS custom_world_config (
    config_key      TEXT        PRIMARY KEY,
    format          TEXT        NOT NULL DEFAULT 'table',
    payload         JSONB       NOT NULL,
    source_file     TEXT,
    loaded_at       TIMESTAMPTZ NOT NULL DEFAULT now()
);
