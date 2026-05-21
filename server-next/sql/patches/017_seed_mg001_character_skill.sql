-- QA seed test/mg001: MG combat only (30 skills), compact skill_slot 1..30 for client picker.
-- Master passives: F3 53 tree (not wired) — not in this seed.
-- Apply: ./scripts/db/reset-mg001-skills.sh

BEGIN;

DELETE FROM character_skill
WHERE account_login = 'test' AND character_name = 'mg001';

UPDATE character_roster
SET key_configuration = NULL, updated_at = now()
WHERE account_login = 'test' AND character_name = 'mg001';

INSERT INTO character_skill (account_login, character_name, skill_slot, skill_type, skill_level)
VALUES
    ('test', 'mg001', 1, 1, 20),
    ('test', 'mg001', 2, 2, 20),
    ('test', 'mg001', 3, 3, 20),
    ('test', 'mg001', 4, 4, 20),
    ('test', 'mg001', 5, 5, 20),
    ('test', 'mg001', 6, 7, 20),
    ('test', 'mg001', 7, 8, 20),
    ('test', 'mg001', 8, 9, 20),
    ('test', 'mg001', 9, 10, 20),
    ('test', 'mg001', 10, 11, 20),
    ('test', 'mg001', 11, 12, 20),
    ('test', 'mg001', 12, 13, 20),
    ('test', 'mg001', 13, 14, 20),
    ('test', 'mg001', 14, 17, 20),
    ('test', 'mg001', 15, 18, 20),
    ('test', 'mg001', 16, 19, 20),
    ('test', 'mg001', 17, 20, 20),
    ('test', 'mg001', 18, 21, 20),
    ('test', 'mg001', 19, 22, 20),
    ('test', 'mg001', 20, 23, 20),
    ('test', 'mg001', 21, 39, 20),
    ('test', 'mg001', 22, 41, 20),
    ('test', 'mg001', 23, 47, 20),
    ('test', 'mg001', 24, 55, 20),
    ('test', 'mg001', 25, 56, 20),
    ('test', 'mg001', 26, 57, 20),
    ('test', 'mg001', 27, 73, 20),
    ('test', 'mg001', 28, 76, 20),
    ('test', 'mg001', 29, 236, 20),
    ('test', 'mg001', 30, 237, 20);

COMMIT;

SELECT count(*) AS mg001_skills
FROM character_skill
WHERE account_login = 'test' AND character_name = 'mg001';
