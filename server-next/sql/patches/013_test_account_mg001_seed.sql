-- QA seed: MG character mg001 on account `test` — lv400 Duel Master (3rd class),
-- set Thánh Thần MG +15 (380, no helm), Kiếm Thánh Thần (MG) 0,58 +15, Cánh Lôi Vũ (12,39) +15,
-- full MG skill list on join (server F3 11), 50k stat points, 2B warehouse zen.
-- Apply: ./scripts/apply-sql.sh "$TAKUMI_PG_CONNECTION_STRING" sql/patches/013_test_account_mg001_seed.sql
-- Disconnect in-game before apply so a live session does not overwrite rows.

BEGIN;

-- 2B zen in shared warehouse (account.warehouse_zen), not character zen.
UPDATE account
SET warehouse_zen = 2000000000
WHERE account_login = 'test';

INSERT INTO character_roster (
    account_login, character_name, server_class, level, experience,
    map_id, pos_x, pos_y, angle,
    current_hp, max_hp, current_mp, max_mp, zen,
    current_shield, max_shield,
    strength, dexterity, vitality, energy, leadership, level_up_point,
    current_bp, max_bp, updated_at
)
VALUES (
    'test', 'mg001', 120, 400, 3822148080,
    0, 130, 125, 6,
    0, 0, 0, 0, 0,
    0, 0,
    800, 600, 300, 250, 0, 50000,
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
    'test', 'mg001', 120, 400, 3822148080,
    0, 130, 125, 6,
    0, 0, 0, 0, 0,
    0, 0,
    800, 600, 300, 250, 0, 50000,
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
WHERE account_login = 'test' AND character_name = 'mg001';

-- Wear (ItemWire602 / Season 6): 0,58 Kiếm Thánh Thần (MG); 7,39 Cánh Lôi Vũ (MG 380 tier-3);
-- NOT 12,36 Cuồng Phong (DK), NOT 12,37 Thiên Sứ (DW).
-- 8–11,142 Thánh Thần MG set; no helm. NOT 5,54 Khuyển (SUM) nor 0,57 DK Thánh Thần kiếm.
INSERT INTO inventory_slot (account_login, character_name, slot_idx, item, updated_at) VALUES
    ('test', 'mg001', 0, decode('3AFFFF7F0000000000000000', 'hex'), now()),
    ('test', 'mg001', 7, decode('27FFFF7F00C0000000000000', 'hex'), now()),
    ('test', 'mg001', 3, decode('8EFFFF7F0080000000000000', 'hex'), now()),
    ('test', 'mg001', 4, decode('8EFFFF7F0090000000000000', 'hex'), now()),
    ('test', 'mg001', 5, decode('8EFFFF7F00A0000000000000', 'hex'), now()),
    ('test', 'mg001', 6, decode('8EFFFF7F00B0000000000000', 'hex'), now());

COMMIT;

SELECT account_login, warehouse_zen FROM account WHERE account_login = 'test';

SELECT character_name, server_class, level, experience, level_up_point, zen,
       strength, dexterity, vitality, energy
FROM character_roster
WHERE account_login = 'test' AND character_name = 'mg001';

SELECT slot_idx, encode(item, 'hex') AS item_hex
FROM inventory_slot
WHERE account_login = 'test' AND character_name = 'mg001'
ORDER BY slot_idx;
