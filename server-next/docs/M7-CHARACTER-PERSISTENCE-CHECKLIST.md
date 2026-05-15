# M7 — Character persistence lifecycle (HP / MP / zen / map / tile)

**Quy ước:** chỉ tick `[x]` khi đã có trong git và có thể chứng minh (test hoặc QA ghi rõ). Cập nhật file này khi merge.

**Phụ thuộc:** **`docs/M4-TILE-AND-COORDINATES.md`** (tile `byte`), **`docs/M4-ROSTER-SSOT.md`** (JSON + mirror), **`docs/M6-GAME-TCP-CHECKLIST.md`** (GameHost minimal-login). **Không chặn M5** (join/ticket).

**Tham chiếu client / legacy:** `Source/4.GameServer` save/load nhân vật; `WSclient.h` join stats (`PRECEIVE_JOIN_MAP_SERVER`); M1 map opcode sau vào thế giới.

---

## M7a — Schema & migration

- [x] `sql/init/004_character_roster_vitals.sql` — thêm `current_hp`, `max_hp`, `current_mp`, `max_mp`, `zen` trên `public.character_roster` (`ALTER … IF NOT EXISTS`).
- [x] Ghi chú trong `README.md` / `apply-sql.sh` header nếu thứ tự file thay đổi (hiện `001` → `004` theo tên).
- [ ] (Tuỳ chọn) Cột `shield` / `skill_mana` nếu join wire cần parity đầy đủ hơn — đối chiếu `JoinMapServerWire602` offsets.

---

## M7b — Domain model (C#)

- [x] Mở rộng **`CharacterRosterRow`** + **`PostgresCharacterRosterRepository`** (`Load` / `Replace`) để đọc/ghi vitals (0 = “chưa set”).
- [x] Đồng bộ **`GameRosterEntry`** (JSON `takumi-roster`) + **`CharacterRosterEntry`** / **`RosterPersistChar`** trong **`LegacyLoginHost`** (camelCase JSON).
- [x] Migration path file JSON cũ: thiếu field → default 0; không làm vỡ `ApplyLegacySpawnIfUnset`.
- [x] **`CharacterRosterRowMapping.ToRow`** + merge overlay vitals trên login (`LegacyLoginHost`, `GamePortMinimalSession`).

---

## M7c — Join wire (F3 03) từ dữ liệu đã lưu

- [x] **`JoinMapStatWire.FromRoster`** + **`CharacterRosterVitals`**: khi `max_hp` / `max_mp` &gt; 0 dùng giá trị đã lưu; ngược lại stub theo class/level; `zen` &gt; 0 ghi vào offset gold.
- [x] DB overlay sau login copy vitals cùng map/xy (`ApplyDbOverlay`); JSON thiếu field vẫn 0.
- [x] Mục “vitals overlay” trong **`docs/M4-ROSTER-SSOT.md`**.

---

## M7d — Vòng đời phiên (save triggers)

- [x] Lưu vitals khi **disconnect** (cùng `SavePersistedRoster` / `SaveRoster` — đã ghi field vitals; cần seed trước, xem dưới).
- [x] **Seed sau join / move-map:** `JoinMapVitalsSeed` + `RosterVitalsLifecycle` — copy HP/MP/zen từ **`F3 03`** vào roster khi `max_hp == 0`; `rosterDirty` cho **`TAKUMI_ROSTER_PERIODIC_SAVE_SECONDS`**.
- [x] **Cả hai host:** `LegacyLoginHost` + `GamePortMinimalSession`.
- [ ] Cập nhật vitals **giữa phiên** từ gói client hoặc khi GS gửi `0x26`/`0x27` — *minimal host không nhận life/mana từ client (M1: server TX); cần `GCLifeSend` parity trong **M6+** / combat.*

---

## M7e — Kiểm thử

- [x] Unit test: JSON vitals — **`GameRosterVitalsJsonTests`**; seed join — **`JoinMapVitalsSeedTests`**.
- [x] `TEST_PG_CONNECTION_STRING`: vitals round-trip — **`CharacterRosterPostgresVitalsTests`** (cần `004_character_roster_vitals.sql` đã apply).

---

## Liên kết milestone khác

| Milestone | Checklist |
|-----------|-----------|
| M8 — ETL thế giới tĩnh | **`docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`** §M8 |
| M9 — NPC / monster | cùng file §M9 |
| M10 — Movement + broadcast | cùng file §M10 |
