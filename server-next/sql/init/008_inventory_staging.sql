-- M7: optional ETL source for inventory_slot (flat legacy ItemIndex → 12-byte wire via importer).
-- Populate via external SQL/CSV; importer: TAKUMI_IMPORT_INVENTORY_STAGING=1

CREATE TABLE IF NOT EXISTS inventory_staging (
    account_login   TEXT        NOT NULL,
    character_name  TEXT        NOT NULL,
    slot_idx        SMALLINT    NOT NULL CHECK (slot_idx >= 0 AND slot_idx <= 255),
    item_index      INTEGER     NOT NULL CHECK (item_index >= 0),
    item_level      SMALLINT    NOT NULL DEFAULT 0,
    durability      SMALLINT    NOT NULL DEFAULT 255,
    skill           BOOLEAN     NOT NULL DEFAULT false,
    luck            BOOLEAN     NOT NULL DEFAULT false,
    item_option     SMALLINT    NOT NULL DEFAULT 0,
    excellent       SMALLINT    NOT NULL DEFAULT 0,
    PRIMARY KEY (account_login, character_name, slot_idx)
);

CREATE INDEX IF NOT EXISTS idx_inventory_staging_character
    ON inventory_staging (account_login, character_name);
