-- QA: allow testing Fire Slash orb (ITEM_WING+16) on mg001 by removing pre-seeded skill 55.
-- Re-apply 017_seed_mg001_character_skill.sql to restore full MG kit.

DELETE FROM character_skill
WHERE account_login = 'test'
  AND character_name = 'mg001'
  AND skill_type = 55;
