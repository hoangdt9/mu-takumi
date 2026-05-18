-- QA seed: DK Blade Master (3rd class) dk400 on account `test1` — lv400,
-- full DK Hoàng Long 380 +15: 0,22 x2 Đao Quyền Năng, 7–11,46 armor, 12,36 Cánh Cuồng Phong (W3 DK),
-- 50k stat points, 2B character zen (join gold / inventory wallet).
-- Login: account **test1** / password **test1** (not test/test).
-- After apply, run: ./scripts/export-roster-json-from-db.sh test1
--   (legacy-login + game-host also read takumi-roster/test1.json when present)
-- Requires: sql/patches/015_character_key_configuration.sql (or sql/init/013) on DB first.
-- Apply: ./scripts/apply-sql.sh "$TAKUMI_PG_CONNECTION_STRING" ./sql/patches/016_test1_dk400_blade_master_seed.sql
-- Logout in-game before apply.

BEGIN;

INSERT INTO account (account_login, password_hash, security_code, phone, warehouse_zen, updated_at)
VALUES (
    'test1',
    '$2a$11$fxo6Ln/1zq0s4Pd/.Godcu5vIWZIzdLGjK9I.lRFtlWpCJYvPSG1O',
    '',
    '',
    0,
    now()
)
ON CONFLICT (account_login) DO UPDATE SET
    password_hash = EXCLUDED.password_hash,
    updated_at = now();

INSERT INTO character_roster (
    account_login, character_name, server_class, level, experience,
    map_id, pos_x, pos_y, angle,
    current_hp, max_hp, current_mp, max_mp, zen,
    current_shield, max_shield,
    strength, dexterity, vitality, energy, leadership, level_up_point,
    current_bp, max_bp, updated_at
)
VALUES (
    'test1', 'dk400', 56, 400, 3822148080,
    0, 130, 125, 6,
    12000, 12000, 500, 500, 2000000000,
    0, 0,
    2000, 1500, 500, 200, 0, 50000,
    0, 0, now()
)
ON CONFLICT (account_login, character_name) DO UPDATE SET
    server_class = EXCLUDED.server_class,
    level = EXCLUDED.level,
    experience = EXCLUDED.experience,
    map_id = EXCLUDED.map_id,
    pos_x = EXCLUDED.pos_x,
    pos_y = EXCLUDED.pos_y,
    angle = EXCLUDED.angle,
    current_hp = EXCLUDED.current_hp,
    max_hp = EXCLUDED.max_hp,
    current_mp = EXCLUDED.current_mp,
    max_mp = EXCLUDED.max_mp,
    zen = EXCLUDED.zen,
    current_shield = EXCLUDED.current_shield,
    max_shield = EXCLUDED.max_shield,
    strength = EXCLUDED.strength,
    dexterity = EXCLUDED.dexterity,
    vitality = EXCLUDED.vitality,
    energy = EXCLUDED.energy,
    leadership = EXCLUDED.leadership,
    level_up_point = EXCLUDED.level_up_point,
    current_bp = EXCLUDED.current_bp,
    max_bp = EXCLUDED.max_bp,
    updated_at = now();

INSERT INTO character_domain (
    account_login, character_name, server_class, level, experience,
    map_id, pos_x, pos_y, angle,
    current_hp, max_hp, current_mp, max_mp, zen,
    current_shield, max_shield,
    strength, dexterity, vitality, energy, leadership, level_up_point,
    current_bp, max_bp, updated_at
)
VALUES (
    'test1', 'dk400', 56, 400, 3822148080,
    0, 130, 125, 6,
    12000, 12000, 500, 500, 2000000000,
    0, 0,
    2000, 1500, 500, 200, 0, 50000,
    0, 0, now()
)
ON CONFLICT (account_login, character_name) DO UPDATE SET
    server_class = EXCLUDED.server_class,
    level = EXCLUDED.level,
    experience = EXCLUDED.experience,
    map_id = EXCLUDED.map_id,
    pos_x = EXCLUDED.pos_x,
    pos_y = EXCLUDED.pos_y,
    angle = EXCLUDED.angle,
    current_hp = EXCLUDED.current_hp,
    max_hp = EXCLUDED.max_hp,
    current_mp = EXCLUDED.current_mp,
    max_mp = EXCLUDED.max_mp,
    zen = EXCLUDED.zen,
    current_shield = EXCLUDED.current_shield,
    max_shield = EXCLUDED.max_shield,
    strength = EXCLUDED.strength,
    dexterity = EXCLUDED.dexterity,
    vitality = EXCLUDED.vitality,
    energy = EXCLUDED.energy,
    leadership = EXCLUDED.leadership,
    level_up_point = EXCLUDED.level_up_point,
    current_bp = EXCLUDED.current_bp,
    max_bp = EXCLUDED.max_bp,
    updated_at = now();

DELETE FROM inventory_slot
WHERE account_login = 'test1' AND character_name = 'dk400';

-- Wear: 0,22 x2 Đao Quyền Năng (+15); 7–11,46 Sét Hoàng Long DK 380; 12,36 Cánh Cuồng Phong (W3 DK).
INSERT INTO inventory_slot (account_login, character_name, slot_idx, item, updated_at) VALUES
    ('test1', 'dk400', 0, decode('16FFFF7F0000000000000000', 'hex'), now()),
    ('test1', 'dk400', 1, decode('16FFFF7F0000000000000000', 'hex'), now()),
    ('test1', 'dk400', 2, decode('2EFFFF7F0070000000000000', 'hex'), now()),
    ('test1', 'dk400', 3, decode('2EFFFF7F0080000000000000', 'hex'), now()),
    ('test1', 'dk400', 4, decode('2EFFFF7F0090000000000000', 'hex'), now()),
    ('test1', 'dk400', 5, decode('2EFFFF7F00A0000000000000', 'hex'), now()),
    ('test1', 'dk400', 6, decode('2EFFFF7F00B0000000000000', 'hex'), now()),
    ('test1', 'dk400', 7, decode('24FFFF7F00C0000000000000', 'hex'), now());

COMMIT;

SELECT account_login FROM account WHERE account_login = 'test1';

SELECT character_name, server_class, level, level_up_point, zen,
       strength, dexterity, vitality, energy
FROM character_roster
WHERE account_login = 'test1' AND character_name = 'dk400';

SELECT slot_idx, encode(item, 'hex') AS item_hex
FROM inventory_slot
WHERE account_login = 'test1' AND character_name = 'dk400'
ORDER BY slot_idx;
