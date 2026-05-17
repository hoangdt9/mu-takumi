-- Dev QA: grant 2,000,000,000 zen to every character on account `test`.
-- Apply: psql "host=127.0.0.1 port=54444 user=takumi password=takumi dbname=takumi_runtime" -v ON_ERROR_STOP=1 -f sql/patches/012_test_account_zen_2b.sql
-- Tip: disconnect in-game first so a running session does not flush zen=0 back over this row.

BEGIN;

UPDATE character_roster
SET zen = 2000000000, updated_at = now()
WHERE account_login = 'test';

UPDATE character_domain
SET zen = 2000000000, updated_at = now()
WHERE account_login = 'test';

COMMIT;

SELECT account_login, character_name, level, zen
FROM character_roster
WHERE account_login = 'test'
ORDER BY character_name;
