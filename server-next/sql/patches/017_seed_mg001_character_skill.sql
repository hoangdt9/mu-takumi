-- Seed mg001 MG skill list into character_skill (parity MagicListWire602 / F3 11).
-- Apply after 016_character_skill.sql.

BEGIN;

DELETE FROM character_skill
WHERE account_login = 'test' AND character_name = 'mg001';

INSERT INTO character_skill (account_login, character_name, skill_slot, skill_type, skill_level)
VALUES
    ('test', 'mg001', 1, 1, 20),
    ('test', 'mg001', 2, 2, 20),
    ('test', 'mg001', 3, 3, 20),
    ('test', 'mg001', 4, 4, 20),
    ('test', 'mg001', 5, 5, 20),
    ('test', 'mg001', 7, 7, 20),
    ('test', 'mg001', 8, 8, 20),
    ('test', 'mg001', 9, 9, 20),
    ('test', 'mg001', 10, 10, 20),
    ('test', 'mg001', 11, 11, 20),
    ('test', 'mg001', 12, 12, 20),
    ('test', 'mg001', 13, 13, 20),
    ('test', 'mg001', 14, 14, 20),
    ('test', 'mg001', 17, 17, 20),
    ('test', 'mg001', 18, 18, 20),
    ('test', 'mg001', 19, 19, 20),
    ('test', 'mg001', 20, 20, 20),
    ('test', 'mg001', 21, 21, 20),
    ('test', 'mg001', 22, 22, 20),
    ('test', 'mg001', 23, 23, 20),
    ('test', 'mg001', 41, 41, 20),
    ('test', 'mg001', 47, 47, 20),
    ('test', 'mg001', 55, 55, 20),
    ('test', 'mg001', 56, 56, 20),
    ('test', 'mg001', 57, 57, 20),
    ('test', 'mg001', 73, 73, 20),
    ('test', 'mg001', 236, 236, 20),
    ('test', 'mg001', 237, 237, 20);

COMMIT;

SELECT count(*) AS mg001_skills
FROM character_skill
WHERE account_login = 'test' AND character_name = 'mg001';
