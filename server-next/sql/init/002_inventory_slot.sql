-- M4b+: per-character inventory rows for Season 6 F3 10 (12-byte item blobs, same as GameServer ItemByteConvert wire).
-- Apply on existing DBs: ./scripts/apply-sql.sh "$URI"  (runs all sql/init/*.sql in order)

CREATE TABLE IF NOT EXISTS inventory_slot (
    account_login   TEXT        NOT NULL,
    character_name  TEXT        NOT NULL,
    slot_idx        SMALLINT    NOT NULL CHECK (slot_idx >= 0 AND slot_idx <= 255),
    item            BYTEA       NOT NULL CHECK (octet_length(item) = 12),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (account_login, character_name, slot_idx)
);

CREATE INDEX IF NOT EXISTS idx_inventory_slot_lookup ON inventory_slot (account_login, character_name);

-- Example (QA): replace `item` with a valid 12-byte Season 6 wire blob (GameServer ItemByteConvert).
-- INSERT INTO inventory_slot (account_login, character_name, slot_idx, item)
-- VALUES ('admin', 'HERO', 12, decode('0000000000000000000000000000', 'hex'));

