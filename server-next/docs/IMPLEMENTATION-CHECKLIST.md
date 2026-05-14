# Takumi Server Next - Implementation Checklist

Last updated: 2026-05-14 (M2 Protocol extraction + golden tests)

## Repository vs checklist (read first)

- **`server-next/README.md`** describes what is actually in tree: **`server-next/docker-compose.yml`** starts **PostgreSQL** and **LegacyLoginHost** in Docker (Connect **44605**, login **44606**; Postgres **54444** by default). When connect env vars are unset, **LegacyLoginHost** sends a **multi-ID F4 06** default so `ServerList.bmd` usually shows sub-servers (group = connectId/20); override with **`TAKUMI_CS_CONNECT_IDS`** or **BASE+COUNT**. The .NET host can still be run on the host with `dotnet watch` when you prefer hot reload — **do not** bind the same ports while Docker publishes them.
- The **`## Done`** section below is the **intended / previously implemented** feature set. Treat unchecked **Exit criteria** and **`## In Progress`** as the current engineering truth for QA; do not assume every `[x]` is verifiable without a compilable solution in git.
- **Native client (C++) session notes**
  - Character select (touch → `StartGame`, ray pick, IME sau login): **`../../docs/DEVELOPMENT-LOG-2026-05-12.md`**
  - IME toàn cục / modal / xóa nhân vật (captcha 6 số phía client), JNI **Done** → Return, `UpdateMouseFromTouch` trước handler: **`../../docs/DEVELOPMENT-LOG-2026-05-14.md`**

## Client APK, `data.zip`, and Docker (what to redo when)

Use this to avoid unnecessary rebuilds.

| Change | Rebuild & reinstall APK? | Re-download `data.zip`? | Docker / ops |
|--------|--------------------------|-------------------------|----------------|
| Only `server-next` C#, compose, `.env`, `env.defaults`, SQL, keys on host | **No** | **No** | `docker compose` **up / recreate** the stack; `TAKUMI_PUBLIC_HOST` + URLs in **`.env`** (see `.env.lan.example`). Recreate legacy-login if you changed `.env` after first `up`. |
| Takumi client under `Source/5.Main` (C++), `GameConfigConstants`, JNI bootstrap | **Yes** (if you touched native) | Only if zip contract/content changed | APK **Connect/bootstrap + `data.zip` LAN URL** are baked from **`server-next/.env`** at Gradle configure time (`BuildConfig`); rebuild after `.env` IP/URL change. |
| In-client **server list / IP** edited in UI (if the build supports it) | **No** | **No** | Ensure Docker publishes the addresses the client will use after sub-select. |

**`data.zip` / Preload:** the Android preload step usually **skips download** when `Android/data/.../files/Data` is already valid and the ready marker exists—no need to re-fetch for pure server-side iteration unless you wipe app storage or change the bundle URL/content.

**Parallel stacks:** if both `takumi-openmu` and `server-next` run, keep **host ports and client target** unambiguous (e.g. OpenMU `44505` vs Takumi `44605`) so QA logs match the stack under test.

**Minimal Docker on Mac (Android QA):** for APK pointed at `server-next`, run **`cd server-next && docker compose up -d`** (or **`./scripts/docker-up.sh`**) — **Postgres** (default **54444**) and **LegacyLoginHost** (**44605** / **44606**) with **`TAKUMI_LAN_IP`** in `.env` = Mac LAN IP. **LAN `data.zip`:** `docker compose --profile datazip up -d` or **`./scripts/docker-up.sh --with-datazip`** (nginx on host **18080**, same `takumi/docker/data-zip/host/data.zip`); do **not** also run `takumi/docker` `datazip` on the same publish port. **Stop** the `takumi-openmu` compose group (and `docker` Wine/SQL profiles) while testing `server-next` to avoid port confusion and extra emulation load. See **`../../docs/ANDROID-DEV-MAC.md`** § *Takumi Server Next* and **`../README.md`**.

## Done

- [x] Scaffold modular solution:
  - `Takumi.Server.Domain`
  - `Takumi.Server.Protocol`
  - `Takumi.Server.Connect`
  - `Takumi.Server.Login`
  - `Takumi.Server.Persistence`
  - `Takumi.Server.Host`
- [x] Host starts Connect + Login as hosted services.
- [x] Runtime EF Core persistence wired with PostgreSQL.
- [x] Auto-apply migrations on startup.
- [x] Seed default account + character on empty database (`admin/admin` -> `ADMIN`).
- [x] Add runtime character table/entity in `takumi_runtime.character`.
- [x] Persist base character stats (str/agi/vit/ene/hp/mp) and use them in post-select join payload.
- [x] Add startup importer from `takumi_legacy` staging tables into runtime account/character tables.
- [x] Add CSV batch ETL script for loading `takumi_legacy` staging tables.
- [x] Add packet hex tracing for RX/TX logs.
- [x] Connect packet handling:
  - [x] `C1 05` (PatchInfo)
  - [x] `C1 F4 02/06` (ServerList)
  - [x] `C1 F4 03` (ServerInfo)
- [x] Login/character-select packet handling:
  - [x] `F1 01` login request parsing (username/password from fixed fields)
  - [x] `F1 01` login response (`C1 F1 01`)
  - [x] `F3 00` character list request response (DB-backed; Takumi `PHEADER_DEFAULT_CHARACTER_LIST` + `PRECEIVE_CHARACTER_LIST` 33 bytes/slot)
  - [x] `F3 01` character create request response (persist to DB)
  - [x] `F3 02` character delete request response (persist to DB)
  - [x] `F3 15` focus character request response
  - [x] `F3 03` select character: `CharacterFocused` (`F3 15`) then full Takumi `PRECEIVE_JOIN_MAP_SERVER` (**C1**, length **131**, `#pragma pack(1)` layout per `WSclient.h`) — not OpenMU `CharacterInformation075` (42-byte `C3`).
  - [x] After join payload, send Season 6 `F3 10` inventory from `inventory_slot` on the same login TCP session.
  - [x] Keep session open and process multiple packets on one TCP stream
  - [x] Enforce authenticated-only character-list/select flow
- [x] Encrypted `C3`/`C4` client→server decrypt path: `Season6ClientToServerDecryptSession` (per-connection counter; 12×`uint` key optional, OpenMU-compatible fallback when empty).
- [x] Login-port compatibility for clients that query server list / server info on the login TCP port:
  - answer `F4 06 / F4 03 / PatchInfo` on login socket (same payloads as connect server)
- [x] Integration tests for login/select baseline (including multi-packet burst, fragmented reads, unknown-packet ignore):
  - [x] login + character list (offsets for 8+33 list layout)
  - [x] select flow: `F3 15` + **131-byte** `F3 03` join map + `F3 10` inventory
  - [x] reject when not authenticated (`F3 00`)
  - [x] reject unknown character select (`F3 03`)
- [x] **Reproducible LAN / dev env (no committed machine IP):**
  - [x] Committed **`server-next/env.defaults`** (ports, join version, serial, `TAKUMI_ACCOUNTS` format) + gitignored **`server-next/.env`** from **`.env.lan.example`** (`YOUR_LAN_IP`).
  - [x] **`RepoEnvLoader`**: load `env.defaults` (fill unset) then `.env` (override) before `LegacyLoginHost` reads configuration.
  - [x] **Android Gradle** (`Source/android/app/build.gradle`): `BuildConfig.DATA_ZIP_URL_LAN` + `MU_BOOTSTRAP_SERVER_*` from **`../../server-next/.env`**; optional `-P` / `TAKUMI_EMULATOR_DATA_ZIP_URL`; **`gradle.properties`** keeps Mac VPN/timeout knobs only (no bootstrap IP).
  - [x] **`docker-compose.yml`**: `TAKUMI_LOGIN_PORT` / `TAKUMI_CONNECT_PORT` interpolated from host `.env` with compose defaults.
  - [x] **Docs:** `../../docs/ANDROID-DEV-MAC.md` (new Mac checklist), `../README.md` (env.defaults row), `run-legacy-login-host.sh` sources `env.defaults` then `.env`.

## In Progress

- [ ] Validate packet parity against real Takumi client captures (golden pcap loop).
- [ ] Confirm Connect→Login→Select→in-game on real Android client (no black screen after `LoadWorld`). **Requires** Docker + LAN IP correct; **requires** APK rebuilt from current `Source/5.Main` for **touch-to-enter** after character select (`SEASON3B::IsPress` / long-press path in `ZzzScene.cpp` — see repo `docs/DEVELOPMENT-LOG-2026-05-12.md`). Rebuild again after IME/modal fixes logged **`docs/DEVELOPMENT-LOG-2026-05-14.md`**.
- [ ] Load SimpleModulus CS decrypt key from `keys/Dec2.dat` (or `TAKUMI_SIMPLEMODULUS_CS_DEC_KEY_PATH`) into `Season6ClientToServerDecryptSession` instead of hardcoded fallback only.

## Next (High Priority)

- [ ] **M4–M5:** Persist **map + X/Y + angle** + join **ticket / game-port handoff** (see checklist **§ Lộ trình chuẩn**).  
- [ ] **M6:** **`Takumi.Server.Game`** + **`Takumi.Server.GameHost`** — TCP listener + decrypt loop (parity `Source/4.GameServer` bootstrap); *not in this branch* (merge with `main` removed experimental projects). Planned: `dotnet run` on dedicated game port (e.g. **55901**, `TAKUMI_GAME_PORT`).  
- [ ] Dedicated **game** TCP server after character select (minimal map/scope packets) if client leaves login socket or expects game port.
- [ ] Persist and **enforce** session ticket / account session state in `TakumiLoginServer` (table `session_ticket` exists; not wired yet).
- [ ] Extend integration tests: malformed C1 length, oversized packet close, rate limits.

## Next (Medium Priority)

- [ ] Add explicit error/reason codes for rejected character requests.
- [ ] Add request-rate safeguards (max packet size / protocol violation close may already exist — confirm and document).
- [ ] Improve observability: structured logs with packet code/subcode fields; metrics for auth and packet types.
- [ ] Separate compatibility mode flags by client build/version.

## Data & Migration Follow-up

- [x] Define character schema/entities for real list/select behavior.
- [x] Add ETL/staging import pipeline entrypoint from `takumi_legacy` tables at host startup.
- [x] Map staging data into runtime schema (`takumi_runtime`) for account + character domain (startup importer).
- [x] Runtime `inventory_slot` table + staging `inventory_staging` + startup importer (minimal item fields).
- [x] After `F3 03`, send Season 6 `F3 10` inventory from `inventory_slot` (extended item encoding; flat `ItemIndex` → group/number).
- [ ] Extend staging→runtime mapping to skills, warehouse, guild/social domains.

## Lộ trình chuẩn — `server-next` chạy tương đương `Source/` (đánh số module)

**Mục tiêu:** Client `Source/5.Main` (và bản build Android) vẫn dùng **cùng giao thức / cổng / luồng** như khi nối tới stack C++ legacy (`Source/1.ConnectServer`, `3.JoinServer`, `4.GameServer`, `2.DataServer`, `6.GetMainInfo`), nhưng **dịch vụ .NET** trong `server-next` thay thế dần từng executable — không nhân đôi logic trong `Program.cs` một file.

**Ánh xạ nhanh `Source/*.sln` → module `server-next`:**

| Legacy (`Source/`) | Vai trò | Module / project đích (ưu tiên chuẩn) |
|--------------------|---------|--------------------------------------|
| `1.ConnectServer` | Danh sách server, patch info | **M2** `Takumi.Server.Connect` *(hoặc host gom)* — hiện gói trong `LegacyLoginHost` |
| `3.JoinServer` | Chọn sub-server, chuyển client sang cổng GS | **M4** `Takumi.Server.Join` — ticket / chuyển TCP |
| `4.GameServer` | Thế giới, quái, NPC, combat, lưu tọa độ | **M5–M9** `Takumi.Server.Game` (+ domain persistence) |
| `2.DataServer` | Cache / truy vấn SQL cho GS | **M10** `Takumi.Server.Persistence` + adapter, hoặc gộp Postgres |
| `6.GetMainInfo` | Version / download list | **M11** endpoint hoặc gói trong Connect |
| `5.Main` | Client | **Không** thuộc `server-next`; chỉ dùng để **golden pcap / parity test** |

---

### Các bước có thứ tự (implement dần)

1. **M1 — Giao thức & tài liệu parity** *(baseline done — extend when `TranslateProtocol` / `ProtocolCoreEx` / host changes)*  
   - [x] Liệt kê opcode / `HeadCode` / `F3` sub mà client `Source/5.Main` dùng sau login (**`WSclient.cpp` → `TranslateProtocol`**, macro TX trong **`wsclientinline.h`**, struct trong **`WSclient.h`**) + lớp **`Protocol.cpp` → `ProtocolCoreEx`**. Chi tiết: **`docs/M1-PROTOCOL-PARITY-MAP.md`**.  
   - [x] Bảng “legacy vs server-next” + wire login ngắn: **`docs/M1-PROTOCOL-PARITY-MAP.md`** §1 và **`docs/LOGIN-WIRE-FORMAT.md`**. *(GameServer C++ đầy đủ nằm ngoài `server-next`; khi thêm vector từ `Source/4.GameServer`, bổ sung mục “GS TX” trong file M1.)*

2. **M2 — `Takumi.Server.Protocol` (shared)** *(done 2026-05-14 — extend when join/list wire changes)*  
   - [x] Tách struct/builder join, inventory, list nhân vật khỏi `LegacyLoginHost/Program.cs` sang **`src/Takumi.Server.Protocol/`** (`CharacterListWire602`, `JoinMapServerWire602`, `CharacterCreateWire602`, `LoginAccountWire602`, `ConnectServerList602`, `ConnectServerInfo602`, `CharacterRosterWire`, …).  
   - [x] Unit test golden vectors: **`src/Takumi.Server.Tests/CharacterWireGolden602Tests.cs`** (`dotnet test src/Takumi.Server.Tests/Takumi.Server.Tests.csproj`).

3. **M3 — Connect (`1.ConnectServer` parity)**  
   - [ ] Đảm bảo F4 06/03/05 đồng hành với `ServerListManager` / BMD (đã phần nào trong host).  
   - [ ] Tách process hoặc class library nếu cần scale riêng Connect.

4. **M4 — Login + nhân vật + vị trí lưu thế giới**  
   - [ ] Bảng / roster: `map_id`, `pos_x`, `pos_y`, `angle` (không hard-code Lorencia trong join).  
   - [ ] Đồng bộ với client disconnect nếu vẫn một socket — hoặc chuyển sang M5 sau ticket.

5. **M5 — Join handoff (`3.JoinServer` parity)**  
   - [ ] Session ticket ký / timeout; client chuyển sang **cổng game** (hoặc cùng host nhưng trạng thái “in-game”).  
   - [ ] Trả đúng địa chỉ GS mà `ServerList.bmd` / client mong đợi.

6. **M6 — Game TCP host (`Takumi.Server.Game`) — skeleton `4.GameServer`**  
   - [ ] Project `src/Takumi.Server.Game` (class library) + **`Takumi.Server.GameHost`** `dotnet run` listener: accept, SimpleModulus + Xor32 giống login, `BeginReceiveAsync` + log RX (bootstrap; chưa gameplay). *Chưa có trong tree hiện tại — ưu tiên compose `main` + LAN `.env`.*  
   - [ ] Log packet head/sub (`event=decrypted_rx` + optional hex khi `TAKUMI_VERBOSE=1`).

7. **M7 — Persistence vòng đời nhân vật**  
   - [ ] Lưu/khôi phục HP/MP/zen/map/xy khi thoát / periodic save (parity `GameServer` save).  
   - [ ] Migration EF / SQL trong `server-next` (hoặc repo migration riêng).

8. **M8 — Dữ liệu tĩnh thế giới (ETL)**  
   - [ ] Import `MuServer/4.GameServer/Data/Monster/MonsterSetBase*.txt` → bảng spawn.  
   - [ ] Cửa / shop / custom từ `Data/Custom/` + nguồn C++ tham chiếu.

9. **M9 — NPC & monster runtime**  
   - [ ] Spawn theo map; regen; bảng monster id → stats (tối thiểu HP đứng yên).  
   - [ ] Gói scope spawn tới client (opcode theo M1).

10. **M10 — Movement & visibility**  
    - [ ] Nhận move từ client; validate map; broadcast quanh player (stub trước).  
    - [ ] Đồng bộ với M7 để không reset vị trí.

11. **M11 — DataServer merge**  
    - [ ] Quyết định: Postgres-only vs bridge tới MSSQL legacy; API nội bộ cho `Takumi.Server.Game`.

12. **M12 — GetMainInfo / version**  
    - [ ] Parity `6.GetMainInfo` nếu client vẫn gọi HTTP/TCP riêng.

13. **M13 — Vận hành**  
    - [x] `docker-compose.yml`: Postgres + LegacyLoginHost; optional profile **`datazip`** (`./scripts/docker-up.sh --with-datazip`).  
    - [ ] Thêm service **`game`** riêng (TCP game port) *hoặc* mở rộng compose — tùy triển khai sau M6+.  
    - [ ] CI: integration test client script hoặc pcap replay tới cổng đúng.

**Ghi chú:** Cho đến khi **M6+** xong, client có thể vẫn **dính một TCP** như `LegacyLoginHost` hiện tại; “chuẩn” là tách **game port** và process giống `Source/`.

## Exit Criteria for "Login/Select MVP"

**Server-side (provable without new APK):**

- [ ] Host logs / hex trace show correct sequence for a scripted or integration-test client: server list → login → character list → focus/select → join + inventory on one TCP session (or documented game-port handoff).
- [ ] Binary integration tests green in CI for Connect/Login packet set.

**Android log triage (real device, tag `TakumiErrorReport`):**

- After **`TCP session start port=<login/game port>`** (e.g. `44606`), **first** inbound bytes should log as **`[AndroidLogin] recv tcp …`** within milliseconds. **No recv tcp at all** means the listening process on that port did not send data (wrong service on port, or not `Takumi.Server.LegacyLoginHost`). Fix with `lsof -nP -iTCP:<port> -sTCP:LISTEN` and run LegacyLoginHost so console shows **`sent join C1 F1 00`** per connection.
- Only **after** join bytes appear does **`Dec2.dat` mismatch** matter for encrypted `C3` login; see `../README.md` and `../../docs/ANDROID-DEV-MAC.md`.

**Client-side (needs installed APK + device):**

- Login on the wire must be **`C3` / SimpleModulus** (`spe.Send(TRUE)` in `SendRequestLogIn`) then Xor32 inside — same as PC MuMain; see **`LOGIN-WIRE-FORMAT.md`** (this folder). A host that only reads **plain `C1`** will never parse a real client login. Use a DB account that exists (seed is often **`admin` / `admin`**; `test` only works if inserted in DB).
- **Character select (Android/iOS, native):** documented implementation for IME suppression, 3D ray sync, and **double-tap / long-press → `StartGame()`** using `SEASON3B::IsPress` / `IsRepeat` (avoids one-frame skew vs `CInput::Update` order). See **`../../docs/DEVELOPMENT-LOG-2026-05-12.md`**. Further IME / modal / delete flow: **`../../docs/DEVELOPMENT-LOG-2026-05-14.md`**.
- [ ] **Character delete (`F3 02`) on device:** client hiện có bước **captcha 6 số cục bộ** trước khi gửi resident (xem `MsgWin.cpp` / `docs/DEVELOPMENT-LOG-2026-05-14.md`). QA cần xác nhận payload resident sau captcha vẫn khớp kỳ vọng Takumi / `server-next` (độ dài, chỉ số).

- [ ] Real Takumi client can:
  - [ ] request server list
  - [ ] login with real account
  - [ ] receive non-synthetic character list from database
  - [ ] focus/select character
  - [ ] enter game from character screen **without hardware keyboard** (double-tap on hero or ~0.5s hold on 3D viewport after selection — native `ZzzScene.cpp`; rebuild APK)
  - [ ] complete minimal transition after character selection (visible world, not black screen)

---

## Planned next steps (consolidated)

Order is suggested dependency / risk reduction:

1. **Device QA (unblocks “MVP done” perception):** real Android against Docker `server-next` — confirm **recv join** → **Translate F1** → select → **no black screen** after `LoadWorld`; capture `adb logcat` + host logs. Tie to **Exit criteria** above.
2. **SimpleModulus CS key from `keys/Dec2.dat`:** replace hardcoded decrypt fallback in `Season6ClientToServerDecryptSession` (**In Progress** item); verify with same `Dec2.dat` as APK `Data/`.
3. **Golden / pcap parity:** scripted or captured client RX/TX vs host (**In Progress**); extend tests for malformed length / oversized close (**Next High**).
4. **Post-select game TCP:** if client leaves login socket or expects separate game port, minimal game listener (**Next High**) + document port handoff in this checklist + `ANDROID-DEV-MAC.md`.
5. **Session ticket:** wire `session_ticket` persistence + enforcement (**Next High**).
6. **Medium:** error codes for rejected character ops, rate limits, structured logging, client version flags.
