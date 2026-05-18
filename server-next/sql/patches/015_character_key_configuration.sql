-- Skill hotkey / QWER option blob (C1 F3 30), parity legacy OptionData + OpenMU KeyConfiguration.
ALTER TABLE character_roster ADD COLUMN IF NOT EXISTS key_configuration BYTEA;

COMMENT ON COLUMN character_roster.key_configuration IS '30-byte client option blob (skill hotkeys, potion keys, UI flags)';
