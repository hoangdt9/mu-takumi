# Checklist: Migrate server Takumi → stack OpenMU (.NET)

**Mục tiêu:** Giữ **game client** Takumi (Android / `ClientBuild` PC) **không đổi** ban đầu; thay **server** (`Source/1–4.*` Win32) bằng **OpenMU** fork/extend, chạy Docker/.NET trên Mac/Linux/Windows.

**Repo tham chiếu:**

| Vai trò | Đường dẫn |
|---------|-----------|
| Server cũ (C++ Win32) | `takumi/Source/1.ConnectServer` … `4.GameServer` |
| Client (giữ) | `takumi/Source/android`, `takumi/ClientBuild_*` |
| DB / script cũ | `takumi/MuServer/7.DataBase` (`.bak`, `.sql`) |
| **OpenMU (nền mới)** | `/Users/hoangmac/Github/OpenMU` — solution `src/MUnique.OpenMU.sln`, deploy `deploy/all-in-one` |

**Chiến lược tổng thể & stack:** `docs/ops/SERVER-PORT-PLAN.md` (phương án **A — OpenMU**).

**Inventory đầy đủ artifact Takumi (file + manifest):** `docs/migration/TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md` và `docs/manifests/` — làm song song các phase dưới đây để không sót logic/data/path.

**Baseline OpenMU trong repo này:** `OpenMU/docs/Checklist-Server.md`, `OpenMU/QuickStart.md`, Docker `OpenMU/deploy/all-in-one`.

**Snapshot tiến độ (cập nhật khi milestone đổi):**

| Khi | Nội dung |
|-----|----------|
| 2026-05 | Phase 2: MSSQL restore; inspectors + **`regen-mapping-slices.sh`**; **`takumi-etl`** (`preview-login-path`, **`staging-login-path`**, **`staging-login-sync.sh`**) → PG schema **`takumi_staging`**; promote vào **`data`**. OpenMU dùng `--schema data`/`config`; export [`tools/db-migrate/schemas/`](../tools/db-migrate/schemas/) gitignored. |

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

Đọc **[`docs/game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md`](game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md)** (DDL `SQLUp.sql`, map `Character`/`Account` EF, ADR nháp A/B).

- [x] Inventory **`EXEC` + bảng (heuristic)** Data/Join → [`docs/game-spec/TAKUMI-SQL-BACKLOG.md`](game-spec/TAKUMI-SQL-BACKLOG.md).
- [x] Inventory **`MuServer/7.DataBase/SQL Back/*.sql`** → cùng file (mục **SQL Back**); **`SQLUp.sql`** đã tóm DDL + mapping concept trong **`PHASE2-OPENMU-DATA-MODEL-MAP.md`**.
- [ ] Ánh xạ **từng proc/bảng** vào Postgres/OpenMU (**spreadsheet** + EF entity name); **`PHASE2` doc chỉ là khung**.
- [x] **Khảo sát** khái niệm OpenMU (**`Character`**, **`Account`**, schema **`data`** / **`config`** trong Postgres = hằng EF `SchemaNames`) — không thay bằng staging DB.
- [ ] Chọn một trong hai và chốt (ghi vào wiki/ADR repo fork):
  - [ ] **A)** Migrate dữ liệu từ `MuOnline.bak` → Postgres theo **mapping** trường (script ETL từng bảng).
  - [ ] **B)** **Fresh world** OpenMU + chỉ import subset (account, character) qua tool one-off.
- [x] Tooling Phase 2: [`tools/db-migrate/README.md`](tools/db-migrate/README.md) — inspectors, **`regen-mapping-slices.sh`**, **`takumi-etl`** (`check-sources`, `preview-login-path`, **`staging-login-path`**, script **`staging-login-sync.sh`**) + template mapping. **Promote staging → `data.*` OpenMU** và transform mật khẩu/blob vẫn TODO.
- [x] **Dump CSV schema (inspector)** trên dev: sau restore MSSQL ([`docker/sql/restore-muonline.sh`](../docker/sql/restore-muonline.sh) + `.bak`) và Postgres OpenMU — chạy hai tool trong [`tools/db-migrate/README.md`](tools/db-migrate/README.md); lưu dưới `tools/db-migrate/schemas/*.csv` (gitignored). _Đã spike 2026-05._
- [ ] **Đồng bộ mapping + không sót MSSQL:** điền cột trong [`PHASE2-MAPPING-TEMPLATE.csv`](game-spec/PHASE2-MAPPING-TEMPLATE.csv); **ưu tiên giá trị nghiệp vụ từ MSSQL** khi trùng tên (chi tiết **[§0](game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md)**); regen slice qua [`tools/db-migrate/README.md`](tools/db-migrate/README.md) khi DB thay đổi. **Lưu ý:** Postgres OpenMU là **đa schema** (`data`/`config`/…), không phải “multi-language DB”; Phase 4+ có thể **chỉnh lại** ETL/schema sau parity GameServer.
- [ ] **Gate 2:** Có bộ dữ liệu dev đủ login + spawn character trên **OpenMU** (kể cả dữ liệu giả lập).

---

## Phase 3 — Connect / Join / Login (tương thích Takumi client)

- [ ] Xác định client Takumi dùng **cổ Connect** nào (so với 44405/44406 OpenMU). _Tham chiếu snapshot:_ [`docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](protocol/TAKUMI-SERVER-NETWORK-BASELINE.md) (UDP `63001` vs TCP `63000` — cần **pcap**).
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

- [x] Inventory thư mục **`MuServer/4.GameServer/Data/**` về mặt chức năng — [`docs/game-spec/GAMESERVER-DATA-FOLDER-MAP.md`](game-spec/GAMESERVER-DATA-FOLDER-MAP.md) (song song [`TAKUMI-MUSERVER-GAMEDATA-FILES.txt`](manifests/TAKUMI-MUSERVER-GAMEDATA-FILES.txt)).
- [x] Trích **`#define`/season** từ vcxproj / headers (vd. `GAMESERVER_UPDATE`) — **đã ghi:** [`docs/game-spec/SEASON-AND-DEFINES.md`](game-spec/SEASON-AND-DEFINES.md); mở rộng `CUSTOM_*` khi audit từng module.
- [x] Chỉ mục điều phối **head/sub** và entry point Connect / Join-internal / Game (phần lõi **`ProtocolCore`**) → [`docs/protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md`](protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md). Bảng **đầy đủ every case** không thực hiện tại markdown — đọc `Protocol.cpp` / `DSProtocol.cpp` trong IDE hoặc bổ sung script riêng.
- [ ] Trích các **formula** trong comment hoặc hằng số đáng nhớ (damage, EXP, drop rate) vào một file `docs/game-spec/` (Markdown bảng + link file C++ làm chứng cứ).

**4.2 — Nền mạng in-game**

- [ ] Xác nhận game TCP/UDP và port trong `Common.ini` Takumi khớp cấu hình listener OpenMU.
- [ ] Bật/decrypt đúng lớp với Takumi trong fork (nếu khác vanilla OpenMU).
- [ ] Test tối thiểu: client vào map, nhận object list cơ bản (golden compare).

**4.3 — MVP gameplay (Gate 4a)**

- [x] **Skill hotkey persistence (client):** `SaveOptions` / `C1 F3 30` ↔ `ReceiveOption`; ô skill chính trống khi chưa gán slot `0`; Android picker + `ApplySelectedSkillIndex` — chi tiết [`docs/game-spec/SKILL-HOTKEY-PERSISTENCE.md`](game-spec/SKILL-HOTKEY-PERSISTENCE.md).
- [ ] **Skill hotkey E2E:** relog trên **OpenMU** và/hoặc **Takumi DataServer** xác nhận 10 ô + ô chính khớp DB.
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

- [`docs/game-spec/TAKUMI-SQL-BACKLOG.md`](game-spec/TAKUMI-SQL-BACKLOG.md) — `EXEC`/bảng heuristic + SQL Back.
- [`docs/game-spec/PHASE2-MAPPING-TEMPLATE.csv`](game-spec/PHASE2-MAPPING-TEMPLATE.csv) — seed spreadsheet proc/bảng × OpenMU.
- [`tools/db-migrate/README.md`](tools/db-migrate/README.md) — **`takumi-mssql-inspect`** + **`takumi-pg-inspect`**, quy ước ETL.
- [`docker/sql/restore-muonline.sh`](../docker/sql/restore-muonline.sh) — restore **`MuOnline.bak`** vào MSSQL Docker (Phase 2 spike).
- [`docs/game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md`](game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md) — Phase 2 DDL `SQLUp` ↔ OpenMU EF + ADR nháp.
- [`docs/game-spec/DATA-SUB1-DRIFT.md`](game-spec/DATA-SUB1-DRIFT.md) — so sánh `Sub 1/Data` vs `Data`.
- [`docs/game-spec/CONNECT-SERVER-REAL-DRIFT.md`](game-spec/CONNECT-SERVER-REAL-DRIFT.md) — `1.ConnectServer` vs `_real` (INI).
- [`docs/game-spec/GAMESERVER-DATA-FOLDER-MAP.md`](game-spec/GAMESERVER-DATA-FOLDER-MAP.md) — `Data/*` → vùng OpenMU (discovery).
- [`docs/migration/MANIFEST-TRACKER-TEMPLATE.md`](MANIFEST-TRACKER-TEMPLATE.md) — §17 spreadsheet / Issues.
- [`docs/migration/OPERATIONS-MIGRATION-NOTES.md`](OPERATIONS-MIGRATION-NOTES.md) — thứ tự batch, docker, scripts.
- [`docs/protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md`](protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md) — chỉ mục head/sub dispatcher (discovery).
- [`docs/protocol/COMPATIBILITY-MATRIX.md`](protocol/COMPATIBILITY-MATRIX.md) — ma trận gói tin (điền dần).
- [`docs/game-spec/SEASON-AND-DEFINES.md`](game-spec/SEASON-AND-DEFINES.md) — macro EX603 / `Util`.
- [`docs/migration/TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) — **danh sách đầy đủ** file/logic/data cần kiểm tra + manifest trong `docs/manifests/`.
- [`docs/ops/SERVER-PORT-PLAN.md`](SERVER-PORT-PLAN.md) — chiến lược A/B/C và phase tổng quát.
- [`docs/android/ANDROID-DEV-MAC.md`](ANDROID-DEV-MAC.md) — build client test.
- [`docs/game-spec/SKILL-HOTKEY-PERSISTENCE.md`](game-spec/SKILL-HOTKEY-PERSISTENCE.md) — skill hotkey client ↔ `F3 30` ↔ OpenMU `KeyConfiguration` / Takumi option DB.
- ~~`docker/` Wine~~ — chỉ sandbox; **production target** là OpenMU Docker.

_Khi checklist này cập nhật, đánh dấu owner + ngày ở từng phase trong issue tracker._
