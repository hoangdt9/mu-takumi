-- M11: account-wide warehouse zen + cash-shop coin balances (WCoin / Goblin Point).
-- Apply: ./scripts/db/apply-sql.sh "$TAKUMI_PG_CONNECTION_STRING"

ALTER TABLE account ADD COLUMN IF NOT EXISTS warehouse_zen BIGINT NOT NULL DEFAULT 0;
ALTER TABLE account ADD COLUMN IF NOT EXISTS wcoin_c BIGINT NOT NULL DEFAULT 0;
ALTER TABLE account ADD COLUMN IF NOT EXISTS wcoin_p BIGINT NOT NULL DEFAULT 0;
ALTER TABLE account ADD COLUMN IF NOT EXISTS goblin_point BIGINT NOT NULL DEFAULT 0;

COMMENT ON COLUMN account.warehouse_zen IS 'M11: zen stored in vault (shared per account; not character Zen)';
COMMENT ON COLUMN account.wcoin_c IS 'M11: WCoin (cash shop priceType=1)';
COMMENT ON COLUMN account.wcoin_p IS 'M11: WCoinP (priceType=2)';
COMMENT ON COLUMN account.goblin_point IS 'M11: Goblin Point (priceType=3)';
