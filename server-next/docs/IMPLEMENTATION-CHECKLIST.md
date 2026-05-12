# Takumi Server Next - Implementation Checklist

Last updated: 2026-05-12

## Repository vs checklist (read first)

- **`server-next/README.md`** describes what is actually in tree: **PostgreSQL** can be started with **`server-next/docker-compose.yml`**. The .NET host must be run from **restored `src/**/*.csproj` sources** (this clone may only contain `bin/` / `obj/` leftovers with **no `.cs` files** — `dotnet build` will not work until sources return).
- The **`## Done`** section below is the **intended / previously implemented** feature set. Treat unchecked **Exit criteria** and **`## In Progress`** as the current engineering truth for QA; do not assume every `[x]` is verifiable without a compilable solution in git.

## Client APK, `data.zip`, and Docker (what to redo when)

Use this to avoid unnecessary rebuilds.

| Change | Rebuild & reinstall APK? | Re-download `data.zip`? | Docker / ops |
|--------|--------------------------|-------------------------|----------------|
| Only `server-next` C#, compose, `.env`, SQL, keys on host | **No** | **No** | `docker compose` **up / recreate** the stack; set `TAKUMI_PUBLIC_HOST` (and published ports) to the IP the **phone** can reach (same LAN as `44605` / login port). |
| Takumi client under `Source/5.Main` (C++), default IP/port in native code, `PreloadActivity` URL, JNI | **Yes** | Only if you changed what the zip contains or the preload contract | Same as above after APK points to the right host. |
| In-client **server list / IP** edited in UI (if the build supports it) | **No** | **No** | Ensure Docker publishes the addresses the client will use after sub-select. |

**`data.zip` / Preload:** the Android preload step usually **skips download** when `Android/data/.../files/Data` is already valid and the ready marker exists—no need to re-fetch for pure server-side iteration unless you wipe app storage or change the bundle URL/content.

**Parallel stacks:** if both `takumi-openmu` and `server-next` run, keep **host ports and client target** unambiguous (e.g. OpenMU `44505` vs Takumi `44605`) so QA logs match the stack under test.

**Minimal Docker on Mac (Android QA):** for APK pointed at `server-next`, run **Postgres** via **`server-next/docker-compose.yml`** (default host port **54444**), then run the **Takumi host** process on the Mac (or your own container) so it listens on `TAKUMI_CONNECT_PORT` / `TAKUMI_LOGIN_PORT` (**44605** / **44606** by default) with `TAKUMI_PUBLIC_HOST` = Mac LAN IP. Optionally add `takumi/docker` **`--profile datazip`** (port **18080**) when you need LAN `data.zip` download. **Stop** the `takumi-openmu` compose group (and `docker` Wine/SQL profiles) while testing `server-next` to avoid port confusion and extra emulation load. See `docs/ANDROID-DEV-MAC.md` § *Takumi Server Next* and `server-next/README.md`.

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
- [ ] Confirm Connect→Login→Select→in-game on real Android client (no black screen after `LoadWorld`). **Requires** Docker + LAN IP correct; **may require** client rebuild if the fix is in `Source/5.Main` / Java—see table above.
- [ ] Load SimpleModulus CS decrypt key from `keys/Dec2.dat` (or `TAKUMI_SIMPLEMODULUS_CS_DEC_KEY_PATH`) into `Season6ClientToServerDecryptSession` instead of hardcoded fallback only.

## Next (High Priority)

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

## Exit Criteria for "Login/Select MVP"

**Server-side (provable without new APK):**

- [ ] Host logs / hex trace show correct sequence for a scripted or integration-test client: server list → login → character list → focus/select → join + inventory on one TCP session (or documented game-port handoff).
- [ ] Binary integration tests green in CI for Connect/Login packet set.

**Android log triage (real device, tag `TakumiErrorReport`):**

- After **`TCP session start port=<login/game port>`** (e.g. `44606`), **first** inbound bytes should log as **`[AndroidLogin] recv tcp …`** within milliseconds. **No recv tcp at all** means the listening process on that port did not send data (wrong service on port, or not `Takumi.Server.LegacyLoginHost`). Fix with `lsof -nP -iTCP:<port> -sTCP:LISTEN` and run LegacyLoginHost so console shows **`sent join C1 F1 00`** per connection.
- Only **after** join bytes appear does **`Dec2.dat` mismatch** matter for encrypted `C3` login; see `../README.md` and `../../docs/ANDROID-DEV-MAC.md`.

**Client-side (needs installed APK + device):**

- Login on the wire must be **`C3` / SimpleModulus** (`spe.Send(TRUE)` in `SendRequestLogIn`) then Xor32 inside — same as PC MuMain; see **`docs/LOGIN-WIRE-FORMAT.md`**. A host that only reads **plain `C1`** will never parse a real client login. Use a DB account that exists (seed is often **`admin` / `admin`**; `test` only works if inserted in DB).
- [ ] Real Takumi client can:
  - [ ] request server list
  - [ ] login with real account
  - [ ] receive non-synthetic character list from database
  - [ ] focus/select character
  - [ ] complete minimal transition after character selection (visible world, not black screen)
