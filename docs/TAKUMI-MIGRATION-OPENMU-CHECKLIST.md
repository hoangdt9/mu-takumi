# Checklist: Migrate server Takumi → stack OpenMU (.NET)

**Mục tiêu:** Giữ **game client** Takumi (Android / `ClientBuild` PC) **không đổi** ban đầu; thay **server** (`Source/1–4.*` Win32) bằng **OpenMU** fork/extend, chạy Docker/.NET trên Mac/Linux/Windows.

**Repo tham chiếu:**

| Vai trò | Đường dẫn |
|---------|-----------|
| Server cũ (C++ Win32) | `takumi/Source/1.ConnectServer` … `4.GameServer` |
| Client (giữ) | `takumi/Source/android`, `takumi/ClientBuild_*` |
| DB / script cũ | `takumi/MuServer/7.DataBase` (`.bak`, `.sql`) |
| **OpenMU (nền mới)** | `/Users/hoangmac/Github/OpenMU` — solution `src/MUnique.OpenMU.sln`, deploy `deploy/all-in-one` |

**Chiến lược tổng thể & stack:** `docs/SERVER-PORT-PLAN.md` (phương án **A — OpenMU**).

**Inventory đầy đủ artifact Takumi (file + manifest):** `docs/TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md` và `docs/takumi-manifests/` — làm song song các phase dưới đây để không sót logic/data/path.

**Baseline OpenMU trong repo này:** `OpenMU/docs/Checklist-Server.md`, `OpenMU/QuickStart.md`, Docker `OpenMU/deploy/all-in-one`.

---

## Phase 0 — Preflight & quyết định (Gate: GO / NO-GO fork sâu)

- [ ] Clone/sync OpenMU, build local theo `QuickStart` hoặc **`docker compose up -d --no-build`** trong `deploy/all-in-one`.
- [ ] Đăng nhập Admin Panel, Start Connect + Game server (theo `Checklist-Server`).
- [ ] Thử client **tham chiếu** (MuMain / S6) lên OpenMU: login → nhân vật → vào map.
- [ ] Bắt **pcap hoặc log** từ **Takumi client** → **server Win cũ** (Connect / Join / Game) — lưu vào `docs/protocol/captures/` (git LFS hoặc kho riêng nếu file lớn).
- [x] Ghi baseline cổ & shard Takumi vào **`docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`**; **`docs/protocol/COMPATIBILITY-MATRIX.md`** có **khung** (điền sau pcap/OpenMU parity).
- [ ] So sánh cụ thể khung **gói tin / opcode / encrypt** Takumi vs OpenMU/MuMain và cập nhật đầy đủ `COMPATIBILITY-MATRIX.md` + captures trong `docs/protocol/captures/`.
- [ ] **Gate 0:** Quyết có **đủ chồng lấn protocol** để coi OpenMU là nền không. Nếu lệch rất lớn → ghi rõ “cần nhánh plugin/parser riêng trong OpenMU” hoặc reconsider brownfield thuần.
- [ ] Tạo nhánh làm việc: ví dụ `feature/takumi-openmu-baseline` trên **fork OpenMU** (khuyến nghị fork riêng, không sửa nhánh upstream trực tiếp).
- [ ] Thống nhất **version .NET** OpenMU đang dùng (README repo: .NET 10) với máy dev.

---

## Phase 1 — Hạ tầng dev & observability

- [ ] Mỗi dev: `dotnet --version`, Docker, cổ trùng `QuickStart` (44405/44406/55901–55906/…).
- [ ] Chạy được **OpenMU từ source** (ít nhất một máy) để debug (`src/MUnique.OpenMU.sln` + Postgres local).
- [ ] Thiết lập **CI** trên fork (build + unit test) — tái sử dụng workflow Azure/OpenMU nếu có.
- [ ] Quy ước log: Serilog/OpenTelemetry theo chuẩn OpenMU hiện có.
- [ ] Tài liệu “một lệnh”: `docker compose` cho team Takumi (wrapper script hoặc `compose.takumi.yml` mở rộng all-in-one sau này).

---

## Phase 2 — Dữ liệu (SQL Server → mô hình OpenMU / PostgreSQL)

- [x] Inventory **`EXEC` + bảng (heuristic)** Data/Join → [`docs/takumi-game-spec/TAKUMI-SQL-BACKLOG.md`](takumi-game-spec/TAKUMI-SQL-BACKLOG.md).
- [x] Inventory **`MuServer/7.DataBase/SQL Back/*.sql`** → cùng file (mục **SQL Back**).
- [ ] Ánh xạ từng proc/bảng vào **schema OpenMU / Postgres** (spreadsheet + ADR; restore `.bak` làm chân lý DDL).
- [ ] So sánh với **schema OpenMU** (EF migrations, setup DB admin tạo world).
- [ ] Chọn một trong hai (ghi vào ADR ngắn):
  - [ ] **A)** Migrate dữ liệu từ `MuOnline.bak` → Postgres theo **mapping** trường (script ETL từng bảng).
  - [ ] **B)** **Fresh world** OpenMU + chỉ import subset (account, character) qua tool one-off.
- [ ] Tooling: script `sql` / `dotnet` restore + transform — đặt trong repo (vd. `OpenMU/deploy/takumi-migration/` hoặc `takumi/tools/db-migrate/`).
- [ ] **Gate 2:** Có bộ dữ liệu dev đủ login + spawn character trên **OpenMU** (kể cả dữ liệu giả lập).

---

## Phase 3 — Connect / Join / Login (tương thích Takumi client)

- [ ] Xác định client Takumi dùng **cổ Connect** nào (so với 44405/44406 OpenMU).
- [ ] Nếu lệch packet: mở rộng **packet handlers** trong `MUnique.OpenMU.Network` (hoặc plugin) — tham chiếu `OpenMU/docs/Packets/`.
- [ ] Viết **integration test** TCP: golden bytes từ Phase 0 → assert response length/flag tối thiểu.
- [ ] **Gate 3:** Client Takumi (PC hoặc Android) **list server + login** vào được tài khoản (có thể chưa vào map ổn định).

---

**Ước lượng thời gian GameServer:** đây là phần **lớn nhất**. Team nhỏ: **đi được MVP in-game với Takumi client** thường **vài tháng**; **parity gần server cũ** (events, skill custom, ekonomi…) dễ **1 năm trở lên**. Không có đường tắt “migrate xong trong vài tuần” vì không phép dịch máy nguyên bộ `.cpp`.

---

## Phase 4 — GameServer `Source/4.GameServer` ↔ OpenMU (làm dần)

### Port hay viết code mới?

**Không nên và gần như không làm được “port” máy:** chuyển từng file `GameServer/*.cpp` (Win32 MSVC) sang C# trong OpenMU **không** có công cụ đáng tin; coupling WinAPI / ODBC / threading / struct layout sẽ vượt xa lợi ích.

**Chiến lược đúng:** coi codebase Takumi (**`takumi/Source/4.GameServer`**) và **`MuServer/4.GameServer/**/Data`**, **`GameServerInfo - *.ini`**) là:

| Nguồn | Việc dùng trong OpenMU |
|-------|-------------------------|
| **Config / TXT / INI** | So sánh với định nghĩa world của OpenMU; map tham số hoặc import qua tooling. |
| **C++ (.cpp)** | Đọc để hiểu **công thức**, **opcode phản hồi**, edge case → **implement lại** trong `GameLogic`/plugin .NET có test. |
| **Golden packets** | So sánh hành vi client Takumi không cần đọc hết ~280 file. |

**Viết code mới trong OpenMU** (theo behavior), có **benchmark** và **replay** là chuẩn industry cho “migration” kiểu này — không copy-paste dòng legacy trừ từng đoạn công thức sạch có comment nguồn.

---

### Checklist làm dần (ánh xạ từ `4.GameServer` → OpenMU)

**4.1 — Discovery (chỉ đọc/ghi backlog, không sửa engine OpenMU)**

- [ ] Inventory thư mục `GameServer/` (theo chức năng): combat, monster, player, skill, item, map, NPC, castle, vuốt list file chính.
- [ ] Trích **`#define`/season** từ vcxproj / headers (vd. `GAMESERVER_UPDATE`) — gắn label cho fork.
- [ ] Liệt kê **opcode / packet handlers** được đề cập trong code (grep `0x`, `CASE`, send buffer) để ghép vào handlers OpenMU.
- [ ] Trích các **formula** trong comment hoặc hằng số đáng nhớ (damage, EXP, drop rate) vào một file `docs/takumi-game-spec/` (Markdown bảng + link file C++ làm chứng cứ).

**4.2 — Nền mạng in-game**

- [ ] Xác nhận game TCP/UDP và port trong `Common.ini` Takumi khớp cấu hình listener OpenMU.
- [ ] Bật/decrypt đúng lớp với Takumi trong fork (nếu khác vanilla OpenMU).
- [ ] Test tối thiểu: client vào map, nhận object list cơ bản (golden compare).

**4.3 — MVP gameplay (Gate 4a)**

- [ ] Di chuyển + sync nhân vật.
- [ ] Đánh quái / damage / die / respawn tối thiểu.
- [ ] Nhặt / drop đơn giản + inventory đồng bộ packet.
- [ ] Chat hoặc lệnh tối thiểu nếu client bắt buộc.
- [ ] **Gate 4a:** 15–30 phút chơi nội bộ không disconnect do thiếu packet.

**4.4 — Mở rộng nhóm hệ**

- [ ] Skill / cooldown / buff đồng nhất Takumi vs OpenMU (từng skill ưu tiên theo dùng thật).
- [ ] Item options, Excellent, Jewel, Chaos mix (nhóm theo mức độ monetization của server).
- [ ] Map/monster spawn từ `MonsterSetBase`, `Terrain` và tương đương OpenMU — tool convert hoặc nhập tay có script.
- [ ] Dungeon / event (Blood Castle, Devil Square, …) theo backlog ưu tiên doanh thu/người chơi.
- [ ] Guild, alliance, siege — sau core combat.
- [ ] Shop / zen / warehouse / PCafe nếu custom.

**4.5 — Tin cậy & khớp cũ**

- [ ] Bảng regression: scenario từ server cũ → kỳ vọng ↔ OpenMU (HP, damage snapshot, loot table mẫu).
- [ ] Load test nhẹ vài chục connection; không memory leak trong vài giờ.
- [ ] **Gate 4b:** QA với nhóm nhỏ dùng **client Takumi thật** trên staging.

---

## Phase 4 (tóm Gate)

- [ ] **Gate 4:** Gameplay cơ bản với **client Takumi** ngang mức “chơi được nội bộ” trên OpenMU fork (đủ sau khi xong MVP + một phần 4.4 tùy product owner).

---

## Phase 5 — Đóng gói & vận hành

- [ ] Docker image / compose cho môi trường Takumi (DB init + OpenMU + nginx nếu cần).
- [ ] Hướng dẫn IP (nhớ client MU thường **không dùng 127.0.0.1** — dùng `127.127.127.127` hoặc IP LAN theo `QuickStart`).
- [ ] Backup/restore Postgres, rotation log, thay mật khẩu admin.
- [ ] **Gate 5:** Staging chạy 24–48h không leak memory nghiêm trọng, có smoke test tự động.

---

## Phase 6 — Cleanup server cũ

- [ ] Ngừng triển khai `MuServer` Win32 trong môi trường chính thức.
- [ ] Giữ `Source/1–4` trong repo chỉ làm **tham chiếu** (hoặc archive branch).
- [ ] Cập nhật README Takumi + OpenMU fork: đường build, link doc protocol.

---

## Rủi ro & chủ đề luôn mở

| Rủi ro | Việc cần làm |
|--------|----------------|
| Protocol / encrypt không khới | Spike Phase 0; có thể cần **nhánh chỉ patch Network** không merge upstream dễ. |
| Logic game không có trong OpenMU | Implement trong `GameLogic` / plugin hoặc tạm “custom handler” có test. |
| Client phụ thuộc bug server cũ | Ghi nhận; quyết sửa client hay emulate behavior (chỉ khi không tránh được). |

---

## Liên kết nội bộ Takumi

- [`docs/takumi-game-spec/TAKUMI-SQL-BACKLOG.md`](takumi-game-spec/TAKUMI-SQL-BACKLOG.md) — 62 `EXEC` + bảng heuristic (Data/Join C++).
- [`docs/takumi-game-spec/DATA-SUB1-DRIFT.md`](takumi-game-spec/DATA-SUB1-DRIFT.md) — so sánh `Sub 1/Data` vs `Data`.
- [`docs/takumi-game-spec/GAMESERVER-VS-GAMESERVER-REAL.md`](takumi-game-spec/GAMESERVER-VS-GAMESERVER-REAL.md) — `4.GameServer` vs `4.GameServer_real` (authority + tuning).
- [`docs/OPERATIONS-MIGRATION-NOTES.md`](OPERATIONS-MIGRATION-NOTES.md) — thứ tự batch, docker, scripts.
- [`docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](protocol/TAKUMI-SERVER-NETWORK-BASELINE.md) — cổng / hai shard Takumi snapshot.
- [`docs/protocol/COMPATIBILITY-MATRIX.md`](protocol/COMPATIBILITY-MATRIX.md) — ma trận gói tin (điền dần).
- [`docs/takumi-game-spec/SEASON-AND-DEFINES.md`](takumi-game-spec/SEASON-AND-DEFINES.md) — macro EX603 / `Util`.
- [`docs/TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) — **danh sách đầy đủ** file/logic/data cần kiểm tra + manifest trong `docs/takumi-manifests/`.
- [`docs/SERVER-PORT-PLAN.md`](SERVER-PORT-PLAN.md) — chiến lược A/B/C và phase tổng quát.
- [`docs/ANDROID-DEV-MAC.md`](ANDROID-DEV-MAC.md) — build client test.
- ~~`docker/` Wine~~ — chỉ sandbox; **production target** là OpenMU Docker.

_Khi checklist này cập nhật, đánh dấu owner + ngày ở từng phase trong issue tracker._
