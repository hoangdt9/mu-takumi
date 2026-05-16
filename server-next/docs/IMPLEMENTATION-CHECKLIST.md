# Takumi Server Next - Implementation Checklist

Last updated: 2026-05-16 (`main` @ `0fee99a`: M8/M9 gameplay, M7 vitals, M10 presence, M4/M7 migration docs)

**Phân vùng dev (tránh conflict):** **`docs/WORKSTREAM-OWNERSHIP.md`**.  
**M4 + M7 (nhân vật + item, port từ `Source/`):** **`docs/M4-M7-CHARACTER-ITEM-MIGRATION.md`** — owner đề xuất **`mac-m1`**.

## Repository vs checklist (read first)

- **Roster / tọa độ nhân vật (`mapId`, `posX`, `posY`, `angle`) — tại sao lại thấy trong JSON (`takumi-roster/*.json`, ví dụ `test.json`)?** Đây **không phải** “lưu vào code”; đó là **file persistence tối thiểu của M4a** do `LegacyLoginHost` / `GamePortMinimalSession` ghi khi chạy (`TAKUMI_ROSTER_DIR` hoặc mặc định `./takumi-roster/<accountId>.json`). Mục đích: dev/QA nhanh, tái join đúng tile sau walk, **cùng layout** với mirror Postgres `character_roster` khi bật `TAKUMI_ROSTER_DB_SYNC`. **Nguồn sự thật cuối (SSOT) chỉ Postgres** vẫn là hạng mục **mở** — xem **`## Next (High Priority)`** mục **M4b–M5b Postgres-first roster SSOT** và **`docs/M4-WORLD-POSITION-CHECKLIST.md`** §M4b (dòng *SSOT thực tế vẫn là JSON file*) / §Importer. Khi SSOT xong, host sẽ đọc/ghi world position chủ yếu qua DB + domain `character`, JSON (nếu còn) chỉ còn tuỳ chọn import/export hoặc bị loại bỏ.
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

**Minimal Docker on Mac (Android QA):** for APK pointed at `server-next`, run **`cd server-next && docker compose up -d`** (or **`./scripts/docker-stack.sh --detach`** — mặc định kèm profile **datazip**) — **Postgres** (default **54444**) and **LegacyLoginHost** (**44605** / **44606**) with **`TAKUMI_LAN_IP`** in `.env` = Mac LAN IP. **LAN `data.zip`:** `docker compose --profile datazip up -d` or **`./scripts/docker-stack.sh --detach`** (nginx on host **18080**, same `takumi/docker/data-zip/host/data.zip`); do **not** also run `takumi/docker` `datazip` on the same publish port. **Stop** the `takumi-openmu` compose group (and `docker` Wine/SQL profiles) while testing `server-next` to avoid port confusion and extra emulation load. See **`../../docs/ANDROID-DEV-MAC.md`** § *Takumi Server Next* and **`../README.md`**.

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
- [x] Connect packet handling (standalone connect port via `Takumi.Server.Connect`):
  - [x] `C1 05` (PatchInfo)
  - [x] `C1 F4 02/06` (ServerList)
  - [x] `C1 F4 03` (ServerInfo)
- [x] Login/character-select packet handling:
  - [x] `F1 01` login request parsing (username/password from fixed fields)
  - [x] `F1 01` login response (`C1 F1 01`)
  - [x] `F3 00` character list request response (Takumi `PHEADER_DEFAULT_CHARACTER_LIST` + `PRECEIVE_CHARACTER_LIST` 34 bytes/slot in `server-next`; nguồn roster hiện **JSON `takumi-roster/<account>.json` + merge mirror DB** khi bật sync — SSOT Postgres-only: **`## Next (High Priority)`** + **`docs/M4-WORLD-POSITION-CHECKLIST.md`**)
  - [x] `F3 01` character create request response (persist to DB)
  - [x] `F3 02` character delete request response (persist to DB)
  - [x] `F3 15` focus character request response
  - [x] `F3 03` select character: `CharacterFocused` (`F3 15`) then full Takumi `PRECEIVE_JOIN_MAP_SERVER` (**C1**, length **131**, `#pragma pack(1)` layout per `WSclient.h`) — not OpenMU `CharacterInformation075` (42-byte `C3`). **M4:** spawn + minimal stats from roster (`JoinMapSpawnWire`, `JoinMapServerWire602`).
  - [x] After join payload, send Season 6 `F3 10` on the same TCP session (`InventoryListWire602` / `JoinInventoryPacket602`): **empty** when Postgres mirror off; when **`TAKUMI_ROSTER_DB_SYNC`** + table **`inventory_slot`** (`sql/init/002_inventory_slot.sql`), load **12-byte** `item` blobs per slot for the selected character (`LegacyLoginHost`, `GamePortMinimalSession`).
  - [x] Keep session open and process multiple packets on one TCP stream
  - [x] Enforce authenticated-only character-list/select flow
- [x] Encrypted `C3`/`C4` client→server decrypt path: **`Season6ClientToServerDecryptSession`** (facade) → **`Dec2ServerDecryptKeysLoader`** / `PipelinedDecryptor` (per-connection counter; keys from **`Dec2.dat`**, env **`TAKUMI_DEC2_PATH`**, **`TAKUMI_SIMPLEMODULUS_CS_DEC_KEY_PATH`**, OpenMU-compatible fallback when missing).
- [x] Login-port compatibility for clients that query server list / server info on the login TCP port:
  - answer `F4 06 / F4 03 / PatchInfo` on login socket (same payloads as connect server)
- [x] Integration tests for login/select baseline (including multi-packet burst, fragmented reads, unknown-packet ignore):
  - [x] login + character list (offsets for 8+33 list layout)
  - [x] select flow: `F3 15` + **131-byte** `F3 03` join map + **`F3 10`** inventory (empty or DB-backed; same flush batch)
  - [x] reject when not authenticated (`F3 00`)
  - [x] reject unknown character select (`F3 03`)
- [x] **M6 (scaffold + split-port minimal-login):** `Takumi.Server.Game` — `GameListenHost` (**minimal-login** …); **`Takumi.Server.GameHost`** + **`TakumiPostgresMirror`**; **`docs/M6-GAME-TCP-CHECKLIST.md`**. **Đã có:** `[event=decrypted_rx]`, mirror **`character_roster`**, `F3 10` từ DB, optional **`session_ticket`** + **`TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF`**, **signed ticket on wire** (`F1 A5` / `F1 A6`, **`TAKUMI_GAME_TICKET_WIRE`**, **`TAKUMI_SESSION_TICKET_HMAC_KEY`**). **Còn thiếu:** SSOT Postgres-only cho roster (tách khỏi JSON).
- [x] **Reproducible LAN / dev env (no committed machine IP):**
  - [x] Committed **`server-next/env.defaults`** (ports, join version, serial, `TAKUMI_ACCOUNTS` format) + gitignored **`server-next/.env`** from **`.env.lan.example`** (`YOUR_LAN_IP`).
  - [x] **`RepoEnvLoader`**: load `env.defaults` (fill unset) then `.env` (override) before `LegacyLoginHost` reads configuration.
  - [x] **Android Gradle** (`Source/android/app/build.gradle`): `BuildConfig.DATA_ZIP_URL_LAN` + `MU_BOOTSTRAP_SERVER_*` from **`../../server-next/.env`**; optional `-P` / `TAKUMI_EMULATOR_DATA_ZIP_URL`; **`gradle.properties`** keeps Mac VPN/timeout knobs only (no bootstrap IP).
  - [x] **`docker-compose.yml`**: `TAKUMI_LOGIN_PORT` / `TAKUMI_CONNECT_PORT` interpolated from host `.env` with compose defaults.
  - [x] **Docs:** `../../docs/ANDROID-DEV-MAC.md` (new Mac checklist), `../README.md` (env.defaults row), `run-legacy-login-host.sh` sources `env.defaults` then `.env`.

## In Progress

- [ ] Validate packet parity against real Takumi client captures (golden pcap loop).
- [ ] Confirm Connect→Login→Select→in-game on real Android client (no black screen after `LoadWorld`). **Requires** Docker + LAN IP correct; **requires** APK rebuilt from current `Source/5.Main` for **touch-to-enter** after character select (`SEASON3B::IsPress` / long-press path in `ZzzScene.cpp` — see repo `docs/DEVELOPMENT-LOG-2026-05-12.md`). Rebuild again after IME/modal fixes logged **`docs/DEVELOPMENT-LOG-2026-05-14.md`**.
- [x] Load SimpleModulus CS decrypt key from `keys/Dec2.dat` via **`Dec2ServerDecryptKeysLoader`** / **`Season6ClientToServerDecryptSession`** (`TAKUMI_DEC2_PATH`, **`TAKUMI_SIMPLEMODULUS_CS_DEC_KEY_PATH`**). Post-decrypt **DoS guards:** **`TAKUMI_MAX_DECRYPTED_PACKET_BYTES`** (default 12288) + **`TAKUMI_MAX_PACKETS_PER_SECOND`** (0=off) close TCP on violation (`LegacyLoginHost`, `GamePortMinimalSession`).

## Next (High Priority)

- [x] **M4b observability:** `CharacterRosterMirrorHealth` — merge/upsert success+fail counts; upsert errors log `[roster-health] …` snapshot; **`TAKUMI_ROSTER_HEALTH_LOG`** logs snapshot after **`TryDrainPendingUpserts`** (`docs/M4-WORLD-POSITION-CHECKLIST.md`).
- [x] **M4b SSOT (minimal hosts):** `TAKUMI_ROSTER_DB_PRIMARY`, `character_domain` mirror, `character_staging` importer — **`docs/M4-WORLD-POSITION-CHECKLIST.md`**, **`docs/M4-ROSTER-SSOT.md`**. **M11 (partial):** warehouse/trade/guild/skill stubs trên minimal hosts — **`docs/M11-SOCIAL-WAREHOUSE-SKILLS.md`**; EF full `Takumi.Server.Host` vẫn backlog.
- [ ] **M6+ / M7–M10:** Full **`Takumi.Server.Game`** protocol after join (map/scope/combat) when client uses **only** game TCP — **`docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`**, **`docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`**.
- [x] **Split-stack login handoff (Postgres):** `sql/init/003_session_ticket.sql` + **`PostgresSessionHandoffRepository`**; **`TAKUMI_SESSION_HANDOFF_DB=1`** → `LegacyLoginHost` persists pending row after F1 01; optional **`TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF=1`** on **GameHost** consumes one row before F1 01 success (**IP match** default on, override with **`TAKUMI_GAME_HANDOFF_MATCH_IP=0`**). Optional **signed wire path:** **`TAKUMI_SESSION_TICKET_HMAC_KEY`** + **`TAKUMI_GAME_TICKET_WIRE=1`** — Legacy pushes **`F1 A5`**, client **`F1 A6`** before game **`F1 01`**, consume on attach (see **`docs/M6-GAME-TCP-CHECKLIST.md`**). Plain IP handoff does not send ticket bytes on wire.
- [x] **M5 split processes:** `Takumi.Server.LoginHost` + `Takumi.Server.ConnectHost` (shared `LegacyLoginHost.Runner` / `ConnectServerHostRunner`); Docker profile **`splitstack`**; scripts `run-login-host.sh` / `run-connect-host.sh`. Combined `LegacyLoginHost` unchanged for default Docker.
- [x] **Protocol tests (partial):** `C1DeclaredLength602` guards + **`SessionHandoffPostgresTests`** when **`TEST_PG_CONNECTION_STRING`**. **TCP hardening (partial):** max decrypted frame + optional **`DecryptedPacketRateGate`** (`DecryptedPacketSafety602Tests`). Still open: golden pcap loop, transport-level framing audit vs OpenMU `Connection`, richer rate policy.

## Next (Medium Priority)

- [ ] Add explicit error/reason codes for rejected character requests.
- [x] **Request-rate / frame size (partial):** **`TAKUMI_MAX_DECRYPTED_PACKET_BYTES`** + **`TAKUMI_MAX_PACKETS_PER_SECOND`** (TCP close on `LegacyLoginHost` / `GamePortMinimalSession`); full protocol violation matrix + metrics still open.
- [ ] Improve observability: structured logs with packet code/subcode fields; metrics for auth and packet types.
- [ ] Separate compatibility mode flags by client build/version.

## Data & Migration Follow-up

- [x] Define character schema/entities for real list/select behavior.
- [x] Add ETL/staging import pipeline entrypoint from `takumi_legacy` tables at host startup.
- [x] Map staging data into runtime schema (`takumi_runtime`) for account + character domain (startup importer).
- [x] Runtime **`inventory_slot`** table (`sql/init/002_inventory_slot.sql`) + **`PostgresInventorySlotRepository`** / **`JoinInventoryPacket602`** (12-byte `item` wire blobs; apply SQL on existing volumes via **`./scripts/apply-sql.sh`**).
- [ ] Staging **`inventory_staging`** + startup importer (flat `ItemIndex` → 12-byte encoding / parity `ItemByteConvert`).
- [x] After `F3 03` (and move-map restub), send Season 6 **`F3 10`** from **`inventory_slot`** when **`TAKUMI_ROSTER_DB_SYNC`** is on; otherwise empty list.
- [~] Extend staging→runtime mapping to skills, warehouse, guild/social domains — **M11:** `warehouse_slot` + moves; trade/guild/skill list stubs (**`docs/M11-SOCIAL-WAREHOUSE-SKILLS.md`**); full skill DB + guild domain still open.

## Lộ trình chuẩn — `server-next` chạy tương đương `Source/` (đánh số module)

**Mục tiêu:** Client `Source/5.Main` (và bản build Android) vẫn dùng **cùng giao thức / cổng / luồng** như khi nối tới stack C++ legacy (`Source/1.ConnectServer`, `3.JoinServer`, `4.GameServer`, `2.DataServer`, `6.GetMainInfo`), nhưng **dịch vụ .NET** trong `server-next` thay thế dần từng executable — không nhân đôi logic trong `Program.cs` một file.

**Ánh xạ nhanh `Source/*.sln` → module `server-next`:**

| Legacy (`Source/`) | Vai trò | Module / project đích (ưu tiên chuẩn) |
|--------------------|---------|--------------------------------------|
| `1.ConnectServer` | Danh sách server, patch info | **M3** `Takumi.Server.Connect` (+ list builders trong `Takumi.Server.Protocol`; `LegacyLoginHost` hosts TCP) |
| `3.JoinServer` | Chọn sub-server, chuyển client sang cổng GS | **M5** `Takumi.Server.Join` — ticket + **F4 03** advertised port (`TAKUMI_GAME_PORT`) |
| `4.GameServer` | Thế giới, quái, NPC, combat, lưu tọa độ | **M6–M9** `Takumi.Server.Game` (+ domain persistence) |
| `2.DataServer` | Cache / truy vấn SQL cho GS | **M10** `Takumi.Server.Persistence` + adapter, hoặc gộp Postgres |
| `6.GetMainInfo` | Version / download list | **M11** endpoint hoặc gói trong Connect |
| `5.Main` | Client | **Không** thuộc `server-next`; chỉ dùng để **golden pcap / parity test** |

---

### Các bước có thứ tự (implement dần)

1. **M1 — Giao thức & tài liệu parity** *(baseline done — extend when `TranslateProtocol` / `ProtocolCoreEx` / host changes)*  
   - [x] Liệt kê opcode / `HeadCode` / `F3` sub mà client `Source/5.Main` dùng sau login (**`WSclient.cpp` → `TranslateProtocol`**, macro TX trong **`wsclientinline.h`**, struct trong **`WSclient.h`**) + lớp **`Protocol.cpp` → `ProtocolCoreEx`**. Chi tiết: **`docs/M1-PROTOCOL-PARITY-MAP.md`**.  
   - [x] Bảng “legacy vs server-next” + wire login ngắn: **`docs/M1-PROTOCOL-PARITY-MAP.md`** §1 và **`docs/LOGIN-WIRE-FORMAT.md`**. *(GameServer C++ đầy đủ nằm ngoài `server-next`; khi thêm vector từ `Source/4.GameServer`, bổ sung mục “GS TX” trong file M1.)*

2. **M2 — `Takumi.Server.Protocol` (shared)** *(done 2026-05-14 — extend when join/list wire changes)*  
   - [x] Tách struct/builder join, inventory, list nhân vật khỏi `LegacyLoginHost/Program.cs` sang **`src/Takumi.Server.Protocol/`** (`CharacterListWire602`, `JoinMapServerWire602`, `InventoryListWire602`, `CharacterCreateWire602`, `LoginAccountWire602`, `ConnectServerList602`, `ConnectServerInfo602`, `CharacterRosterWire`, …).  
   - [x] Unit test golden vectors: **`CharacterWireGolden602Tests.cs`**, **`JoinInventoryPacket602Tests.cs`**, **`PostgresEnvSmokeTests.cs`** (`TEST_PG_*` opt-in).

3. **M3 — Connect (`1.ConnectServer` parity)** *(done 2026-05-14 — extend when connect wire / BMD rules change)*  
   - [x] Đảm bảo F4 06/03/05 đồng hành với `ServerListManager` / BMD (đã phần nào trong host).  
   - [x] Tách process hoặc class library nếu cần scale riêng Connect (`Takumi.Server.Connect`; xem **`docs/M3-CONNECT-BMD.md`**).

4. **M4 — Login + nhân vật + vị trí lưu thế giới** *(M4a + M4c walk + **M4b mirror** — SSOT Postgres-only + item persist still open; port map: **`docs/M4-M7-CHARACTER-ITEM-MIGRATION.md`**; chi tiết: **`docs/M4-WORLD-POSITION-CHECKLIST.md`**, vitals → M7)*  
   - [x] Bảng / roster: `map_id`, `pos_x`, `pos_y`, `angle` — **M4a:** `takumi-roster/<accountId>.json`; **M4b:** mirror `character_roster` khi `TAKUMI_ROSTER_DB_SYNC`; join dùng `JoinMapSpawnWire` (`LegacyLoginHostRunner` / `GamePortMinimalSession`).  
   - [x] Đồng bộ disconnect / move-map stub / **walk** + **instant move** → flush JSON + DB mirror.  
   - [x] **M4b:** `CharacterRosterMirrorHealth` + **`TAKUMI_ROSTER_HEALTH_LOG`**. Tile-only: **`docs/M4-TILE-AND-COORDINATES.md`**, **`docs/M4-ROSTER-SSOT.md`**.  
   - [x] **`inventory_slot` write** sau shop buy/sell/repair — `InventorySlotMirrorWriter` (đọc `F3 10` đã có từ trước).
   - [x] **M4b SSOT Postgres-only (minimal hosts)** — **`TAKUMI_ROSTER_DB_PRIMARY`**, **`character_domain`** mirror (`TAKUMI_CHARACTER_DOMAIN_SYNC`); item `0x22`–`0x24` + potion **`0x26`** — **`ItemWorldHandler`**.

5. **M5 — Join handoff (`3.JoinServer` parity)** *(partial — see **`docs/M5-JOIN-HANDOFF-CHECKLIST.md`**)*  
   - [x] **F4 03** advertised port (`TAKUMI_GAME_PORT`) + in-memory session ticket TTL / Touch / Revoke.  
   - [x] Postgres **`session_ticket`** pending row + optional GameHost consume (**`TAKUMI_SESSION_HANDOFF_DB`**, **`TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF`**) + signed wire **`F1 A5` / `F1 A6`** (**`TAKUMI_SESSION_TICKET_HMAC_KEY`**, **`TAKUMI_GAME_TICKET_WIRE`**).  
   - [x] Standalone processes: **`Takumi.Server.LoginHost`** + **`Takumi.Server.ConnectHost`** (profile **`splitstack`**); combined **`LegacyLoginHost`** for single-container QA.

6. **M6 — Game TCP host (`Takumi.Server.Game`) — skeleton `4.GameServer`** *(bootstrap + split-port minimal-login — **`docs/M6-GAME-TCP-CHECKLIST.md`**)*  
   - [x] Project `src/Takumi.Server.Game` + **`Takumi.Server.GameHost`**: accept, decrypt, join, **minimal-login** (F1 01 + roster JSON + F3 00 + **F3 01/02** + F3 03 join + move stub + walk roster + **`GamePortKeepAliveRunner`** + F1 02 ack) khi có `TAKUMI_ACCOUNTS` + serial.  
   - [x] Docker profile **`gamehost`** + `TAKUMI_GAME_PUBLISH`.  
   - [x] Structured log **`[event=decrypted_rx]`** (`TAKUMI_VERBOSE` hoặc **`TAKUMI_STRUCTURED_LOG`**); mirror **`character_roster`** khi save (cùng `CharacterRosterMirrorWriter` như Legacy).  
   - [x] Cross-process session ticket on wire / signed (**`SessionTicketWire602`**, **`PostgresSessionHandoffRepository.TryConsumeSignedWireAttachAsync`**); Postgres consume for IP-only handoff remains **`TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF`** (skipped when **`TAKUMI_GAME_TICKET_WIRE`** is on).

7. **M7 — Persistence vòng đời nhân vật** — **`docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`** + port map **`docs/M4-M7-CHARACTER-ITEM-MIGRATION.md`** §B  
   - [x] SQL prep: **`004_character_roster_vitals.sql`**.  
   - [x] **M7b–c:** vitals trên roster + join **`F3 03`**; tests **`JoinMapVitals602Tests`**.  
   - [x] **M7d (partial):** `JoinMapVitalsSeed`, `LifeManaWire602`, `RosterVitalsOutboundTracker`, **`TAKUMI_SEND_LIFE_MANA_AFTER_JOIN`**.  
   - [x] **M7d (minimal hosts):** combat hit/die/revive, HP regen, vitals DB upsert, potion use (`ItemWorldHandler`). **M11 (partial):** warehouse (`warehouse_slot`, NPC 240), trade window (`0x36`–`0x3D`), guild stub ack, empty **`F3 11`** after join — **`docs/M11-SOCIAL-WAREHOUSE-SKILLS.md`**; PvP full parity still open.  
   - [x] **M7d town respawn:** chết trên field → **`C1 F3 04`** (`CharacterRegenWire602` + `MapRespawnCatalog`) tới tile thành Lorencia **(135,122)**; **không** dùng `C1 0x1C flag=0` (client `PRECEIVE_TELEPORT` thiếu `SubCode` → lệch XY kiểu **(122,1)**). Gate vẫn dùng `0x1C flag=1` (`WorldGameplayHandlers`). 
   - [x] **M7 + M4:** `inventory_slot` upsert sau buy/sell/repair — `InventorySlotMirrorWriter`.  
   - [ ] Migration EF bổ sung (nếu dùng song song với `sql/init`).

8. **M8 — Dữ liệu tĩnh thế giới (ETL)** — **`docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`** §M8  
   - [x] **`sql/init/005_monster_spawn.sql`** + **`PostgresMonsterSpawnRepository`** + **`MonsterSpawnDbImporter`** / **`scripts/import-monster-spawn.sh`**.  
   - [x] Runtime: **`TAKUMI_MONSTER_SPAWN_DB=1`** → **`MapMonsterWorld`** đọc Postgres (fallback file).  
   - [x] Gates / shops / Custom: **`006_map_gate_npc_shop_custom.sql`**, ETL + **`MapGateCatalog`** / **`NpcShopCatalog`**; env **`TAKUMI_WORLD_STATIC_DB=1`** (hoặc từng flag `TAKUMI_MAP_GATE_DB`, `TAKUMI_NPC_SHOP_DB`, `TAKUMI_CUSTOM_WORLD_DB`).  
   - [x] Wire handlers: gate teleport `0x1C`, NPC talk → shop `0x31` (`WorldGameplayHandlers`, `MapGateService`, `NpcShopWire602`).

9. **M9 — NPC & monster runtime** *(**`docs/M9-NPC-MONSTER-CHECKLIST.md`**, **`docs/M9-MONSTER-AI-PORT-CHECKLIST.md`**, **`docs/M9-M8-NPC-GAMEPLAY-OWNERSHIP.md`**, **`server-next/test/M9-monster-combat-qa.md`**, **`docs/WORKSTREAM-OWNERSHIP.md`**)*  
   - [x] Spawn theo map từ **MonsterSetBase.txt** + **Monster.txt** (file + **Postgres** khi `TAKUMI_MONSTER_SPAWN_DB=1`).  
   - [x] Gói **`C2 0x13`** scope spawn sau join + incremental on walk (`MonsterViewportWire602`, `MonsterViewportTracker`); **`C1 0x14`** destroy khi rời view.  
   - [x] Regen delay từ `Monster.txt` (`MapMonsterInstance.TryRegen`).  
   - [x] Combat stub: `C1 0x11` hit / `0x19` skill → damage, `C1 0x14` destroy, `C1 0x16` die (`MonsterCombatHandler`); damage trừ Defense từ `Monster.txt`.  
   - [x] Gate / NPC shop / buy-sell-repair stub (`MapGateService`, `WorldGameplayHandlers`, `ShopCommerceHandler`).
   - [~] **M9b AI:** wander/chase/`0xD4`/`0x18`, monster→player dmg từ `Monster.txt` (+ defense stub theo level), periodic viewport 1s, regen broadcast — **`docs/M9-MONSTER-AI-PORT-CHECKLIST.md`**.  
   - [~] **M9c:** `ItemValue.txt` + `GCItemValueSend` (`C2 F3 E9`), AoE `0xDB`, PvP stub, quest NPC dialog stub (P4.4 partial).  
   - [ ] Element/exp/invasion (P3.2–P4), pathfinding BFS đầy đủ — **M9b P2.3+**.

10. **M10 — Movement & visibility** — **`docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`** §M10; owner: **`docs/WORKSTREAM-OWNERSHIP.md`**  
    - [x] Walk / instant move → roster tile (`LegacyLoginHost`, `GamePortMinimalSession`).  
    - [x] **M10a:** broadcast `C1 0x15` / `0x18` + rate-limit (`GameMapPresenceRegistry`, `TAKUMI_PRESENCE_MAX_BROADCASTS_PER_SECOND`).  
    - [x] **M10b:** player viewport `C2 0x12` on join + walk view range + `0x14` on leave/disconnect (`PlayerViewportWire602`, `PlayerViewportTracker`, `TAKUMI_PLAYER_VIEWPORT_*`).  
    - [ ] Vitals mid-combat broadcast đầy đủ (M7).

11. **M11 — DataServer merge**  
    - [ ] Quyết định: Postgres-only vs bridge tới MSSQL legacy; API nội bộ cho `Takumi.Server.Game`.

12. **M12 — GetMainInfo / version**  
    - [ ] Parity `6.GetMainInfo` nếu client vẫn gọi HTTP/TCP riêng.

13. **M13 — Vận hành**  
    - [x] `docker-compose.yml`: Postgres + LegacyLoginHost; optional profile **`datazip`** (`./scripts/docker-stack.sh` mặc định; **`--no-datazip`** nếu không cần nginx).  
    - [x] Optional profile **`gamehost`** + **`./scripts/docker-stack.sh --detach --with-gamehost`** (M6 `game-host`).  
    - [ ] CI: integration test client script hoặc pcap replay tới cổng đúng.

**Ghi chú:** Cho đến khi **M6+** xong, client có thể vẫn **dính một TCP** như `LegacyLoginHost` hiện tại; “chuẩn” là tách **game port** và process giống `Source/`.

## Exit Criteria for "Login/Select MVP"

**Server-side (provable without new APK):**

- [ ] Host logs / hex trace show correct sequence for a scripted or integration-test client: server list → login → character list → focus/select → join + inventory on one TCP session (or documented game-port handoff).
- [ ] Binary integration tests green in CI for Connect/Login packet set.

**Android log triage (real device, tag `TakumiErrorReport`):**

- **M6 (split port + GameHost + wire ticket):** quy trình từng bước, logcat, và biến môi trường — **`docs/M6-GAME-TCP-CHECKLIST.md`** § **QA Android (thiết bị thật)**.
- After **`TCP session start port=<login/game port>`** (e.g. `44606`), **first** inbound bytes should log as **`[AndroidLogin] recv tcp …`** within milliseconds. **No recv tcp at all** means the listening process on that port did not send data (wrong service on port, or not `Takumi.Server.LegacyLoginHost`). Fix with `lsof -nP -iTCP:<port> -sTCP:LISTEN` and run LegacyLoginHost so console shows **`sent join C1 F1 00`** per connection.
- Only **after** join bytes appear does **`Dec2.dat` mismatch** matter for encrypted `C3` login; see `../README.md` and `../../docs/ANDROID-DEV-MAC.md`.

**Client-side (needs installed APK + device):**

- Login on the wire must be **`C3` / SimpleModulus** (`spe.Send(TRUE)` in `SendRequestLogIn`) then Xor32 inside — same as PC MuMain; see **`LOGIN-WIRE-FORMAT.md`** (this folder). A host that only reads **plain `C1`** will never parse a real client login. Use a DB account that exists (seed is often **`admin` / `admin`**; `test` only works if inserted in DB).
- **Character select (Android/iOS, native):** documented implementation for IME suppression, 3D ray sync, and **double-tap / long-press → `StartGame()`** using `SEASON3B::IsPress` / `IsRepeat` (avoids one-frame skew vs `CInput::Update` order). See **`../../docs/DEVELOPMENT-LOG-2026-05-12.md`**. Further IME / modal / delete flow: **`../../docs/DEVELOPMENT-LOG-2026-05-14.md`**.
- [ ] **Character delete (`F3 02`) on device:** client hiện có bước **captcha 6 số cục bộ** trước khi gửi resident (xem `MsgWin.cpp` / `docs/DEVELOPMENT-LOG-2026-05-14.md`). QA cần xác nhận payload resident sau captcha vẫn khớp kỳ vọng Takumi / `server-next` (độ dài, chỉ số).

- [ ] Real Takumi client can:
  - [ ] request server list
  - [ ] login with real account
  - [ ] receive non-synthetic character list from database *(hiện roster có thể lấy từ JSON + overlay DB theo `TAKUMI_ROSTER_DB_MERGE_MODE`; “chỉ DB” = sau mục **M4b–M5b SSOT** ở trên)*
  - [ ] focus/select character
  - [ ] enter game from character screen **without hardware keyboard** (double-tap on hero or ~0.5s hold on 3D viewport after selection — native `ZzzScene.cpp`; rebuild APK)
  - [ ] complete minimal transition after character selection (visible world, not black screen)

---

## Planned next steps (consolidated)

Order is suggested dependency / risk reduction:

1. **Device QA (unblocks “MVP done” perception):** real Android against Docker `server-next` — confirm **recv join** → **Translate F1** → select → **no black screen** after `LoadWorld`; capture `adb logcat` + host logs. Tie to **Exit criteria** above.
2. **SimpleModulus CS key from `keys/Dec2.dat`:** **`Season6ClientToServerDecryptSession`** + **`Dec2ServerDecryptKeysLoader`** (`TAKUMI_DEC2_PATH`, **`TAKUMI_SIMPLEMODULUS_CS_DEC_KEY_PATH`**); post-decrypt size/rate limits env — **done** in host paths; verify with same `Dec2.dat` as APK `Data/`.
3. **Golden / pcap parity:** scripted or captured client RX/TX vs host (**In Progress**); extend tests for transport framing vs OpenMU `Connection` (**Next High** — partial: **`C1DeclaredLength602`**, max decrypted bytes + **`TAKUMI_MAX_PACKETS_PER_SECOND`** close).
4. **Post-select game TCP:** split **`Takumi.Server.GameHost`** documented in **`docs/M6-GAME-TCP-CHECKLIST.md`** + `ANDROID-DEV-MAC.md`.
5. **Session ticket / split handoff:** Postgres **`session_ticket`** + optional **`TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF`** + signed wire **`TAKUMI_GAME_TICKET_WIRE`** / **`TAKUMI_SESSION_TICKET_HMAC_KEY`** (**done**); still open: dedicated `TakumiLoginServer` exe.
6. **Medium:** error codes for rejected character ops, rate limits, structured logging, client version flags.
