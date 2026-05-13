# Takumi Server Next - Implementation Checklist

Last updated: 2026-05-13 (world-position root cause + world-data migration plan)

## Repository vs checklist (read first)

- **`server-next/README.md`** describes what is actually in tree: **`server-next/docker-compose.yml`** starts **PostgreSQL** and **LegacyLoginHost** in Docker (Connect **44605**, login **44606**; Postgres **54444** by default). Compose defaults **`TAKUMI_CS_CONNECT_BASE=20`** so F4 06 sub-lines match typical `ServerList.bmd` (group = connectId/20). The .NET host can still be run on the host with `dotnet watch` when you prefer hot reload — **do not** bind the same ports while Docker publishes them.
- The **`## Done`** section below is the **intended / previously implemented** feature set. Treat unchecked **Exit criteria** and **`## In Progress`** as the current engineering truth for QA; do not assume every `[x]` is verifiable without a compilable solution in git.
- **Native client (C++) session notes** (Android/iOS character select: IME, touch → `StartGame`, ray pick): **`../../docs/DEVELOPMENT-LOG-2026-05-12.md`** (from this file: up to repo `takumi/docs/`).
- **What is actually in `server-next` today (git):** mostly **`src/Takumi.Server.LegacyLoginHost`** (`Program.cs`) + Docker Postgres wiring — see **`../README.md` § Source code status**. Sections below that reference a multi-project `Takumi.Server.*` solution describe the **intended** architecture; when those projects are absent from the tree, treat **LegacyLoginHost** behaviour and this checklist’s unchecked items as the live contract.

## Client APK, `data.zip`, and Docker (what to redo when)

Use this to avoid unnecessary rebuilds.

| Change | Rebuild & reinstall APK? | Re-download `data.zip`? | Docker / ops |
|--------|--------------------------|-------------------------|----------------|
| Only `server-next` C#, compose, `.env`, SQL, keys on host | **No** | **No** | `docker compose` **up / recreate** the stack; set `TAKUMI_PUBLIC_HOST` (and published ports) to the IP the **phone** can reach (same LAN as `44605` / login port). |
| Takumi client under `Source/5.Main` (C++), default IP/port in native code, `PreloadActivity` URL, JNI | **Yes** | Only if you changed what the zip contains or the preload contract | Same as above after APK points to the right host. |
| In-client **server list / IP** edited in UI (if the build supports it) | **No** | **No** | Ensure Docker publishes the addresses the client will use after sub-select. |

**`data.zip` / Preload:** the Android preload step usually **skips download** when `Android/data/.../files/Data` is already valid and the ready marker exists—no need to re-fetch for pure server-side iteration unless you wipe app storage or change the bundle URL/content.

**Parallel stacks:** if both `takumi-openmu` and `server-next` run, keep **host ports and client target** unambiguous (e.g. OpenMU `44505` vs Takumi `44605`) so QA logs match the stack under test.

**Minimal Docker on Mac (Android QA):** for APK pointed at `server-next`, run **`cd server-next && docker compose up -d`** (or **`./scripts/docker-up.sh`**) — **Postgres** (default **54444**) and **LegacyLoginHost** (**44605** / **44606**) with `TAKUMI_PUBLIC_HOST` in `.env` = Mac LAN IP. **LAN `data.zip`:** `docker compose --profile datazip up -d` or **`./scripts/docker-up.sh --with-datazip`** (nginx on host **18080**, same `takumi/docker/data-zip/host/data.zip`); do **not** also run `takumi/docker` `datazip` on the same publish port. **Stop** the `takumi-openmu` compose group (and `docker` Wine/SQL profiles) while testing `server-next` to avoid port confusion and extra emulation load. See **`../../docs/ANDROID-DEV-MAC.md`** § *Takumi Server Next* and **`../README.md`**.

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

## In Progress

- [ ] Validate packet parity against real Takumi client captures (golden pcap loop).
- [ ] Confirm Connect→Login→Select→in-game on real Android client (no black screen after `LoadWorld`). **Requires** Docker + LAN IP correct; **requires** APK rebuilt from current `Source/5.Main` for **touch-to-enter** after character select (`SEASON3B::IsPress` / long-press path in `ZzzScene.cpp` — see repo `docs/DEVELOPMENT-LOG-2026-05-12.md`).
- [ ] Load SimpleModulus CS decrypt key from `keys/Dec2.dat` (or `TAKUMI_SIMPLEMODULUS_CS_DEC_KEY_PATH`) into `Season6ClientToServerDecryptSession` instead of hardcoded fallback only.

## Next (High Priority)

- [ ] Dedicated **game** TCP server after character select (minimal map/scope packets) if client leaves login socket or expects game port.
- [ ] **Persist last map + X/Y (and angle)** for each character: extend roster JSON **or** DB row used by join builder; replace hard-coded Lorencia stub in `BuildJoinMapServer602` (see **§ Character world position**).
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

## Character world position — why reconnect always “resets”

**Symptom:** After leaving the game and selecting the same character again, the client appears at a **fixed starter position** (e.g. Lorencia) instead of where you logged off.

**Root cause (current `LegacyLoginHost`):**

1. **`BuildJoinMapServer602` in `../src/Takumi.Server.LegacyLoginHost/Program.cs`** builds `PRECEIVE_JOIN_MAP_SERVER` with **hard-coded** tile coordinates (**135, 122**) and default **map id 0** (Lorencia). The `CharacterRosterEntry` is explicitly **ignored** for layout (`_ = r; // roster ignored for stub`).
2. **Roster JSON** (`TAKUMI_ROSTER_DIR` / `./takumi-roster`) persists **name / class / level only** — not **map index, X, Y, angle**, so there is nowhere to read last world state from.
3. There is **no authoritative game server** in this folder that receives movement / map changes and writes them back to DB on disconnect (contrast: a full GameServer saves `PositionX` / `PositionY` / map on save or logout).

**Mitigations (pick one or combine):**

| Approach | Scope |
|----------|--------|
| **A. Minimal** | Extend roster (or `takumi_runtime.character`) with `map_id`, `pos_x`, `pos_y`, `angle`; update `BuildJoinMapServer602` to emit saved values; optionally bump coords when client sends move packets if you add parsing on the login socket. |
| **B. Correct** | Stand up a **dedicated game TCP server** after `F3 03`, persist world state in Postgres (or legacy SQL), and keep LegacyLoginHost for auth/list only — matches **`## Next (High Priority)`** “Dedicated game TCP server”. |

Until **A** or **B** is implemented, **expect spawn at stub join coordinates** every time.

## World content migration → `server-next` (maps, NPCs, monsters)

**Goal:** Move **data-driven world definitions** out of legacy `MuServer` text/XML and C++ loaders into **versioned, testable** assets consumed by `server-next` (JSON/SQL + C# loaders), then drive **scope / spawn / NPC dialog** packets from that model once a game server exists.

**Authoritative sources in this repo (legacy, read-only references):**

| Domain | Typical paths under repo root |
|--------|--------------------------------|
| Monster spawns (per map) | `MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt`, `MonsterSetBaseCS.txt` (+ `Sub 1/` copies); loader: `Source/4.GameServer/GameServer/MonsterSetBase.cpp` |
| NPC / shop / gate (server-side) | `MuServer/4.GameServer/Data/Custom/` (e.g. `CustomNpcMove.txt`, `CustomNpcCommand.txt`); broader NPC tables vary by fork — search `Source/4.GameServer/GameServer/` for `Npc`, `Shop`, `Gate` |
| Events / gates / terrain-dependent logic | `MuServer/4.GameServer/Data/Event/`, `Data/Custom/` |
| Client-only placement (reference) | `Source/5.Main` OBJS / map loaders — **not** the server-of-record; use for cross-check only |

**Suggested migration phases (checklist):**

- [ ] **Inventory & schema** — Tables or JSON schemas for `world_map`, `monster_spawn` (map_id, mob_id, x, y, dir, count, regen), `npc_spawn` (map_id, npc_id, x, y, type), `gate` / `shop_merchant` as needed; document PK and versioning (e.g. `data_revision`).
- [ ] **ETL** — One-off or repeatable importer: parse `MonsterSetBase*.txt` (+ CS variants) into staging CSV/SQL; validate coordinate ranges per map id; reject duplicates with CI report.
- [ ] **NPC / merchant parity** — Map fork-specific NPC/shop/gate tables (often under `Data/Custom/` or SQL dumps) into the same DB model; link `npc_id` to dialog / item lists where the legacy server references IDs.
- [ ] **Runtime API** — Read-only queries from C# (`IMapCatalog`, `IMonsterSpawnTable`) used by future **GameServer** when building initial scope (`0x12` / `0x13` family, exact opcodes per Season 6 Takumi client).
- [ ] **Wire integration** — Blocked until **Dedicated game TCP server** exists: spawn lists must not be sent on the login-only socket unless the client tolerates it (usually it does not).
- [ ] **Persistence loop** — When game server handles movement / map change, update `character` (or session) row so **§ Character world position** mitigation **A** becomes unnecessary for stub join.

**Exit criteria (world migration “done enough” for MVP game port):**

- [ ] Importer runs in CI or `docker compose` init container; seeded DB contains at least **Lorencia** spawns matching a known-good `MonsterSetBase.txt` row count.
- [ ] Game server (when present) logs `mapId` + spawn batch id at player enter; no hard-coded `(135,122)` in join path for DB-backed characters.

## Exit Criteria for "Login/Select MVP"

**Server-side (provable without new APK):**

- [ ] Host logs / hex trace show correct sequence for a scripted or integration-test client: server list → login → character list → focus/select → join + inventory on one TCP session (or documented game-port handoff).
- [ ] Binary integration tests green in CI for Connect/Login packet set.

**Android log triage (real device, tag `TakumiErrorReport`):**

- After **`TCP session start port=<login/game port>`** (e.g. `44606`), **first** inbound bytes should log as **`[AndroidLogin] recv tcp …`** within milliseconds. **No recv tcp at all** means the listening process on that port did not send data (wrong service on port, or not `Takumi.Server.LegacyLoginHost`). Fix with `lsof -nP -iTCP:<port> -sTCP:LISTEN` and run LegacyLoginHost so console shows **`sent join C1 F1 00`** per connection.
- Only **after** join bytes appear does **`Dec2.dat` mismatch** matter for encrypted `C3` login; see `../README.md` and `../../docs/ANDROID-DEV-MAC.md`.

**Client-side (needs installed APK + device):**

- Login on the wire must be **`C3` / SimpleModulus** (`spe.Send(TRUE)` in `SendRequestLogIn`) then Xor32 inside — same as PC MuMain; see **`LOGIN-WIRE-FORMAT.md`** (this folder). A host that only reads **plain `C1`** will never parse a real client login. Use a DB account that exists (seed is often **`admin` / `admin`**; `test` only works if inserted in DB).
- **Character select (Android/iOS, native):** documented implementation for IME suppression, 3D ray sync, and **double-tap / long-press → `StartGame()`** using `SEASON3B::IsPress` / `IsRepeat` (avoids one-frame skew vs `CInput::Update` order). See **`../../docs/DEVELOPMENT-LOG-2026-05-12.md`**.
- [ ] Real Takumi client can:
  - [ ] request server list
  - [ ] login with real account
  - [ ] receive non-synthetic character list from database
  - [ ] focus/select character
  - [ ] enter game from character screen **without hardware keyboard** (double-tap on hero or ~0.5s hold on 3D viewport after selection — native `ZzzScene.cpp`; rebuild APK)
  - [ ] complete minimal transition after character selection (visible world, not black screen)
