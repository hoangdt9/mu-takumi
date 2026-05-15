# M4 — World position & roster — checklist (chỉ tick [x] khi đã làm xong trong repo)

**Quy ước:** Mỗi dòng `[ ]` / `[x]` phải khớp trạng thái thật trong git. Cập nhật file này khi merge PR / hoàn thành bước.

**Tham chiếu code:** `JoinMapSpawnWire`, `JoinMapServerWire602`; roster JSON + **`TakumiPostgresMirror`**; SQL `001`–`004` trong `sql/init/`. **Hợp đồng tọa độ / SSOT:** `docs/M4-TILE-AND-COORDINATES.md`, **`docs/M4-ROSTER-SSOT.md`**.  
**Migration `Source/` → server-next (M4 item + M7):** **`docs/M4-M7-CHARACTER-ITEM-MIGRATION.md`**.

**Định tuyến M7+ (không thuộc close M4):** HP/MP/zen persist + join wire từ DB/JSON → **`docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`**. ETL thế giới tĩnh, NPC, broadcast scope → **`docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`**. M4 giữ vai trò **roster vị thế + mirror + walk tile**; các mục `[ ]` còn lại ở đây là SSOT Postgres-only / `character` domain / listener GS đầy đủ.

**QA thiết bị (2026-05-14):** `docker compose` + Android LAN — sau khi đi trong map và vào lại, log host cho thấy **`F3 03`** với `xy` khớp roster (ví dụ từ spawn mặc định Lorencia sang tọa độ đã walk).

**M4b health (2026-05-15):** `CharacterRosterMirrorHealth` đếm **merge** (sau login, `LoadByAccountAsync` + `ApplyDbOverlay`) và **upsert** (async `ReplaceAccountRosterAsync`). Mỗi lỗi upsert log kèm snapshot; bật **`TAKUMI_ROSTER_HEALTH_LOG=1`** để sau **`TryDrainPendingUpserts`** (disconnect) in một dòng **`[roster-health] merge_ok=…`** ra stderr.

---

## M4a — Wire + JSON roster (`LegacyLoginHost`)

- [x] `JoinMapSpawnWire` + `JoinMapServerWire602` (131 byte, stats tối thiểu).
- [x] JSON `takumi-roster/<account>.json` (`mapId`, `posX`, `posY`, `angle`).
- [x] Migrate roster cũ (thiếu tọa độ) → default spawn.
- [x] Join / move-map stub / flush khi disconnect.
- [x] Cập nhật tọa độ từ gói **walk** (`0xD4` / `0x10`) + **instant move** (`0x15`) trên cùng TCP login/game minimal (roster in-memory → JSON/DB khi disconnect).

---

## M4b — Postgres + nối JSON ↔ DB

### Schema & vận hành DB

- [x] Script tạo bảng `public.character_roster` (`sql/init/001_character_roster.sql`).
- [x] Script tạo bảng **`public.inventory_slot`** (`sql/init/002_inventory_slot.sql`) — `item` BYTEA đúng 12 byte (wire Season 6).
- [x] Docker Postgres mount `./sql/init` → `/docker-entrypoint-initdb.d` (**chỉ** chạy khi volume DB trống lần đầu).
- [x] Ghi rõ trong `server-next/README.md` (hoặc ops doc) lệnh `psql` áp migration cho **DB đã tồn tại** (volume cũ không chạy lại init).
- [x] (Tuỳ chọn) Job CI / script `scripts/apply-sql.sh` idempotent.

### Code Persistence

- [x] Project `Takumi.Server.Persistence` + package **Npgsql**.
- [x] `PostgresCharacterRosterRepository` (`LoadByAccountAsync`, `ReplaceAccountRosterAsync`, `DeleteCharacterAsync`).
- [x] **`PostgresInventorySlotRepository`** + **`JoinInventoryPacket602`** (đọc `inventory_slot` cho **`F3 10`** sau join / move-map khi `TAKUMI_ROSTER_DB_SYNC`).
- [x] **`TakumiPostgresMirror.InitIfEnabled()`** — khởi tạo cả roster + inventory readers (thay `RosterDbRuntime`).
- [x] `PostgresCharacterRosterRepository.BuildConnectionStringFromEnv()` (`TAKUMI_PG_CONNECTION_STRING` hoặc `TAKUMI_PG_HOST` + port/user/password/database).
- [x] `CharacterRosterMerge.ApplyDbOverlay` (merge DB → bản ghi in-memory theo tên).
- [x] Smoke test gated env: **`TEST_PG_CONNECTION_STRING`** → `PostgresEnvSmokeTests` (`dotnet test`; bỏ qua khi biến trống).

### `LegacyLoginHost` — hành vi nối JSON ↔ DB

- [x] `TAKUMI_ROSTER_DB_SYNC=1` / `true` → `TakumiPostgresMirror.InitIfEnabled()` tạo **`PostgresCharacterRosterRepository`** + **`PostgresInventorySlotRepository`** (cùng connection string).
- [x] Sau login: load JSON → **merge** hàng từ DB khi **`TAKUMI_ROSTER_DB_MERGE_MODE`** không phải `json` (DB overlay `map/xy/angle/level/class` nếu trùng tên); **`json`** = giữ roster từ file JSON.
- [x] Sau mỗi `SavePersistedRoster` (ghi JSON xong): **upsert** async toàn bộ roster account lên DB (`DELETE` account + `INSERT` batch).
- [x] Cấu hình merge **`TAKUMI_ROSTER_DB_MERGE_MODE`**: mặc định / `overlay` = overlay DB sau JSON khi login; **`json`** = bỏ overlay (JSON thắng hoàn toàn).
- [x] `DeleteCharacterAsync` gọi **await** khi xóa nhân vật (`F3 02`) trước `SavePersistedRoster` / `SaveRoster` (sau đó vẫn replace full account — an toàn; có thể tối ưu sau).
- [x] Đợi / flush DB upsert tối thiểu: **`CharacterRosterMirrorWriter.TryDrainPendingUpserts`** sau flush disconnect (`LegacyLoginHost`, `GamePortMinimalSession`).
- [x] Health log / metric: merge fail vs upsert fail count — **`CharacterRosterMirrorHealth`** (`RecordMergeSuccess` / `RecordMergeFail` / `RecordUpsertSuccess` / `RecordUpsertFail`), snapshot trên lỗi upsert + **`TAKUMI_ROSTER_HEALTH_LOG`** sau drain; test **`CharacterRosterMirrorHealthTests`**.

### Importer / bảng `takumi_runtime.character` “chuẩn” — **owner: backlog / M7+ (không chặn M5)**

- [x] Bảng domain **`character_domain`** (`sql/init/007_character_domain.sql`) — mirror world + vitals từ `character_roster` khi **`TAKUMI_CHARACTER_DOMAIN_SYNC=1`** (`PostgresCharacterDomainRepository`, `CharacterDomainMirrorWriter`).
- [x] Importer **`character_staging` → `character_roster` + `character_domain`** khi **`TAKUMI_IMPORT_CHARACTER_STAGING=1`** (`CharacterLegacyWorldImporter`; no-op nếu bảng trống).
- [x] **Quyết định SSOT (tài liệu, M4 freeze):** JSON authoritative + Postgres mirror + overlay merge — không Postgres-only cho tới khi có dự án riêng; **`docs/M4-ROSTER-SSOT.md`**. Dev **M5** có thể làm ticket/handoff trước khi SSOT code xong.
- [x] **Triển khai SSOT Postgres-only (minimal hosts):** **`TAKUMI_ROSTER_DB_PRIMARY=1`** + **`TAKUMI_ROSTER_DB_SYNC=1`** — load `character_roster` / fallback `character_domain` / JSON; mirror domain sau mỗi upsert; JSON cache tùy chọn **`TAKUMI_ROSTER_JSON_EXPORT=1`**. EF `takumi_runtime.character` (full host) vẫn ngoài phạm vi file này.

---

## M4c — Vị trí từ game TCP (phụ thuộc M6–M7)

- [x] `LegacyLoginHost` + `GamePortMinimalSession`: cập nhật roster in-memory từ **walk** `C1 … 0xD4` / `0x10` và **instant move** `C1 05 15` (decode bước đi giống OpenMU); **ghi JSON + DB** vẫn theo `SavePersistedRoster` khi disconnect (và các chỗ save khác).
- [x] Periodic save / save throttled giữa phiên: **`TAKUMI_ROSTER_PERIODIC_SAVE_SECONDS`** (5–3600) → flush JSON + mirror khi có **walk / instant move** (cờ dirty; `LegacyLoginHost` + `GamePortMinimalSession`).
- [x] Chuẩn hóa tile BYTE vs tọa độ float world — **hợp đồng M4:** toàn bộ wire + roster + `character_roster` dùng **tile 0–255** (`byte` / cast `smallint`); float / sub-tile **ngoài phạm vi** roster iteration này; tài liệu **`docs/M4-TILE-AND-COORDINATES.md`** + XML doc `CharacterRosterRow` / `JoinMapSpawnWire` / `ClientWalkPackets602`. **Vitals / zen** trên join wire: **`docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`**.
- [ ] Listener **process** game-only (M6 `GameHost`) parity đầy đủ với `GameServer` (scope, broadcast, …) — **owner: M6+ / M8–M10** — **`docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`** §M9–M10; không gộp vào close M4.

---

## Handoff cho dev **M5** (Join / ticket / cổng game)

M4 **đã đủ** cho: vị trí trên roster, JSON flush, mirror **`character_roster`** / **`inventory_slot`**, walk trên **LegacyLoginHost** + **GamePortMinimalSession**, và **observability** merge/upsert.

**M5 làm tiếp (không chờ M4 SSOT):** `Takumi.Server.Join`, **`TAKUMI_GAME_PORT`**, Postgres **`session_ticket`**, **`TAKUMI_SESSION_HANDOFF_DB`**, optional **`F1 A5`/`F1 A6`** — chi tiết **`docs/M5-JOIN-HANDOFF-CHECKLIST.md`**. Android split TCP smoke: **`docs/M6-GAME-TCP-CHECKLIST.md`**.

**Không nằm trong M5:** bảng `character` “chuẩn” + importer world-only, broadcast scope — **`docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`** + **`docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`**. **Tile vs float:** đã chốt hợp đồng tile-only trong **`docs/M4-TILE-AND-COORDINATES.md`** (M4 không dùng float world).

---

## M5+ (liên quan, không thuộc M4)

- [x] Session ticket + handoff cổng game (**M5**) — **`docs/M5-JOIN-HANDOFF-CHECKLIST.md`** (`TAKUMI_GAME_PORT`, `Takumi.Server.Join`).
- [x] `F3 10` sau join / move-map: **`JoinInventoryPacket602`** đọc **`inventory_slot`** khi DB sync bật; nếu không có bảng hoặc không có row → gói **rỗng** (6 byte). Cột `character_name` phải khớp tên đã chuẩn hoá giống roster (`CharacterRosterMerge.NormaliseName`).

---

## Kiểm thử đã có trong repo

- [x] `dotnet test` — golden wire + `CharacterRosterMergeTests`.
- [x] Test tích hợp Postgres tùy chọn: **`TEST_PG_CONNECTION_STRING`** (xem `PostgresEnvSmokeTests`).

---

## Gợi ý `.env` (Docker compose)

Trong container `legacy-login`, Postgres service name là **`postgres`**, cổng **5432**:

```bash
TAKUMI_ROSTER_DB_SYNC=1
TAKUMI_PG_CONNECTION_STRING=Host=postgres;Port=5432;Username=takumi;Password=takumi;Database=takumi_runtime
```

Trên host `dotnet run` (Postgres publish `54444`):

```bash
TAKUMI_ROSTER_DB_SYNC=1
TAKUMI_PG_HOST=127.0.0.1
TAKUMI_PG_PORT=54444
# Optional: one-line cumulative counters after roster DB upsert drain on disconnect
# TAKUMI_ROSTER_HEALTH_LOG=1
```
