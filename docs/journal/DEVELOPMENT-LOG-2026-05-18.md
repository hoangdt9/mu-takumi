# Development log — 2026-05-18

Ghi tiếp từ **`DEVELOPMENT-LOG-2026-05-17.md`**. Dùng kèm **`server-next/docs/milestones/IMPLEMENTATION-CHECKLIST.md`**.

---

## Android client (`Source/5.Main`)

### F3 00 / F3 10 — equip preview & in-game wear

**Vấn đề:** MG `mg001` hiển thị cánh DK (Cuồng Phong), kiếm trong suốt, ô trang bị đỏ (`canEquip=0`).

| Triệu chứng | Nguyên nhân |
|-------------|-------------|
| Cánh giống DK | `TakumiTier3WingModelForClass` ép MG → `MODEL_WING+36` (Storm) bất kể item; seed sai item 12,36 (DK) thay vì **12,39 Lôi Vũ (MG)** |
| Kiếm `meshs=0` | `TakumiEnsurePreviewWeaponModelLoaded` chỉ gọi `Sword(index+1)` → `Sword59` không tồn tại cho item 0,58 |
| Ô đỏ | Seed STR/DEX quá thấp so với yêu cầu +15 380 |

**Đã làm:**

- **`CharacterListEquipPreview602`** (server): build 17-byte preview từ wear slot 0–8; wing roster nibble = `itemSub - 35` (item 39 → nibble 4).
- **`ZzzCharacter.cpp`:** tier-3 wing mesh theo **item index** (`MODEL_WING + 35 + rosterNibble`); không remap theo class wearer.
- **`TakumiEnsurePreviewWeaponModelLoaded`:** thử `Sword` index / index±1 + tên BMD (`absolute02_sword`, `mastery_sword`, …).
- **`TakumiNormalizeMgWeaponItemIndex`:** MG roster/in-game dùng kiếm 0,58 (không Khuyển SUM 5,54).
- **`WSclient.cpp` `ReceiveInventory`:** sau `F3 10` gọi `SetCharacterClass(Hero)` + log `canEquip` / `[TakumiWear]` trên Android.
- **`TakumiWear` logcat:** armor / weapon / wing type + `NumMeshs` để QA mesh load.

**QA seed `013_test_account_mg001_seed.sql` (account `test`, char `mg001`):**

| Slot | Item |
|------|------|
| 0 | 0,58 Kiếm Thánh Thần (MG) +15 |
| 7 | **12,39 Cánh Lôi Vũ** +15 (`27FFFF7F00C000…`) — **không** 12,36 Cuồng Phong (DK), **không** 12,37 Thiên Sứ (DW) |
| 3–6 | 8–11,142 set Thánh Thần MG +15 |
| Stats | STR 800, DEX 600, VIT 300, ENE 250 |

Apply: `server-next/scripts/db/apply-sql.sh` — thoát game trước khi apply; rebuild APK sau sửa `ZzzCharacter.cpp`.

---

## Server (`server-next`)

### Character list & roster

- **`CharacterListEquipPreview602`**, **`CharacterListPacket602`**, tests **`CharacterListEquipPreview602Tests`**.
- **`CharacterRosterMirrorWriter`:** mirror wear + vitals; F3 00 preview từ `inventory_slot`.
- **`CharacterListWire602`:** tích hợp preview builder.
- **`MagicListWire602`:** chỉnh wire skill list + **`MagicListWire602Tests`**.

### Game host / join

- **`GamePortMinimalSession`:** join map + `F3 10` inventory sau select.
- **`CharacterStatPointHandler`:** batch `F3 06` + tests.
- Viewport/monster: **`MapMonsterScopeSender`**, **`MonsterViewportPeriodicLoop`**, **`MonsterViewerRegistry`** tinh chỉnh.

### Tooling

- **`apply-sql.sh`:** hỗ trợ connection string / user `takumi`.
- **`takumi-inventory/test.json`**, **`takumi-roster/test.json`:** đồng bộ mg001 seed.

---

## Checklist cập nhật

- **`server-next/docs/milestones/IMPLEMENTATION-CHECKLIST.md`** — mục F3 00 equip preview + mg001 QA.
- **`server-next/docs/character/M6-GAME-TCP-CHECKLIST.md`** — F3 10 resync sau join.
- **`server-next/docs/character/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`** — seed QA + stat point.
- **`server-next/docs/character/M4-M7-CHARACTER-ITEM-MIGRATION.md`** — preview parity.

---

## QA Android (thiết bị thật)

1. Thoát nhân vật → apply SQL seed → `docker restart takumi-game-host`.
2. Build APK sau pull commit này.
3. Login `test` / `mg001` — logcat kỳ vọng:
   - `[ReceiveInventory] slot=7 type=6183 canEquip=1` (12×512+39)
   - `[TakumiWear] weapon0 type=58 meshs>0`
   - `[TakumiWear] wing item=6183` (Lôi Vũ)
