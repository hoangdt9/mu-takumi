-- Cross-process login → game TCP handoff (M5/M6). Apply on existing DBs: ./scripts/apply-sql.sh "postgresql://…"
-- One unconsumed row per account at a time (login host replaces on re-login).

CREATE TABLE IF NOT EXISTS session_ticket (
    ticket_id UUID PRIMARY KEY,
    account_login TEXT NOT NULL,
    expires_utc TIMESTAMPTZ NOT NULL,
    issued_utc TIMESTAMPTZ NOT NULL DEFAULT (CURRENT_TIMESTAMP),
    client_ip TEXT NULL,
    consumed_utc TIMESTAMPTZ NULL
);

CREATE INDEX IF NOT EXISTS idx_session_ticket_account_pending
    ON session_ticket (account_login)
    WHERE consumed_utc IS NULL;
