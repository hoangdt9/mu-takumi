-- M11: per-account warehouse (shared across characters on account; parity GameServer Warehouse[]).
CREATE TABLE IF NOT EXISTS warehouse_slot (
    account_login   TEXT        NOT NULL,
    slot_idx        SMALLINT    NOT NULL CHECK (slot_idx >= 0 AND slot_idx <= 255),
    item            BYTEA       NOT NULL CHECK (octet_length(item) = 12),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (account_login, slot_idx)
);

CREATE INDEX IF NOT EXISTS idx_warehouse_slot_lookup ON warehouse_slot (account_login);
