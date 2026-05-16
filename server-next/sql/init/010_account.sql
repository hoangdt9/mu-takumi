-- M14: runtime account credentials (parity legacy MEMB_INFO / OpenMU Account).
-- Login + in-game registration (C1 D3 05) read/write this table when TAKUMI_ACCOUNT_DB=1.

CREATE TABLE IF NOT EXISTS account (
    account_login   TEXT        PRIMARY KEY,
    password_hash   TEXT        NOT NULL,
    security_code   TEXT        NOT NULL DEFAULT '',
    phone           TEXT        NOT NULL DEFAULT '',
    bloc_code       SMALLINT    NOT NULL DEFAULT 0,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_account_phone ON account (phone)
    WHERE phone <> '';
