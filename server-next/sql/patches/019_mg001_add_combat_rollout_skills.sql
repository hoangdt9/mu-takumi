-- Incremental: MG combat/animation QA skills for test/mg001 (SKILL-MATRIX + SkillCombatCatalog).
-- Safe to re-run. Does not delete existing rows (keeps orb-learned skill 55 if present).
-- Full fresh seed: re-apply 017_seed_mg001_character_skill.sql after updating that file.

BEGIN;

INSERT INTO character_skill (account_login, character_name, skill_slot, skill_type, skill_level)
VALUES
    ('test', 'mg001', 48, 48, 20),
    ('test', 'mg001', 49, 49, 20),
    ('test', 'mg001', 50, 50, 20),
    ('test', 'mg001', 51, 51, 20),
    ('test', 'mg001', 52, 52, 20),
    ('test', 'mg001', 61, 61, 20),
    ('test', 'mg001', 62, 62, 20),
    ('test', 'mg001', 63, 63, 20),
    ('test', 'mg001', 64, 64, 20),
    ('test', 'mg001', 65, 65, 20),
    ('test', 'mg001', 238, 238, 20),
    ('test', 'mg001', 385, 385, 20),
    ('test', 'mg001', 482, 482, 20),
    ('test', 'mg001', 487, 487, 20),
    ('test', 'mg001', 490, 490, 20),
    ('test', 'mg001', 493, 493, 20)
ON CONFLICT (account_login, character_name, skill_slot) DO UPDATE SET
    skill_type = EXCLUDED.skill_type,
    skill_level = GREATEST(character_skill.skill_level, EXCLUDED.skill_level),
    updated_at = now();

COMMIT;

SELECT count(*) AS mg001_skills, array_agg(skill_type ORDER BY skill_type) AS types
FROM character_skill
WHERE account_login = 'test' AND character_name = 'mg001';
