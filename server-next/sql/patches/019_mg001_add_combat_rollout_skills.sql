-- Deprecated: rollout skills merged into 017_seed_mg001_character_skill.sql (MG-only 74 skills).
-- Re-run: ./scripts/db/reset-mg001-skills.sh

SELECT count(*) AS mg001_skills
FROM character_skill
WHERE account_login = 'test' AND character_name = 'mg001';
