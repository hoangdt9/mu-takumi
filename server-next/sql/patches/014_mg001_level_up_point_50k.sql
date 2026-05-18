-- Grant 50k unspent stat points to test/mg001 (both roster + domain).
-- Apply while character is OFFLINE (logout), or periodic save will overwrite again.

UPDATE character_roster
SET level_up_point = 50000, updated_at = now()
WHERE account_login = 'test' AND character_name = 'mg001';

UPDATE character_domain
SET level_up_point = 50000, updated_at = now()
WHERE account_login = 'test' AND character_name = 'mg001';

SELECT 'character_roster' AS src, character_name, level_up_point
FROM character_roster
WHERE account_login = 'test' AND character_name = 'mg001'
UNION ALL
SELECT 'character_domain', character_name, level_up_point
FROM character_domain
WHERE account_login = 'test' AND character_name = 'mg001';
