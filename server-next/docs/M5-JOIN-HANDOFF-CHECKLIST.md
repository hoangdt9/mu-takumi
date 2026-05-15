# M5 — Join handoff (Connect → game port) + session ticket

Last updated: 2026-05-15 (split processes: LoginHost + ConnectHost)

## Goal

Align **Takumi `Source/5.Main`** behaviour after Connect Server **`C1 F4 03`** (`ReceiveServerConnect` in `WSclient.cpp`): client closes the connect socket and opens TCP to **advertised IP + port**. For **single-process** `LegacyLoginHost`, the listen port is usually the same as the advertised port; when you split **game** TCP (M6), advertised port can differ from **login listen** port.

## Exit criteria

- [x] **`TAKUMI_GAME_PORT`** optional: if set (1..65535), **`ConnectServerInfo602.Build`** uses it for **F4 03** LE port; if unset, defaults to **`TAKUMI_LOGIN_PORT`**.
- [x] **`Takumi.Server.Join`**: `InMemorySessionTicketStore` — issue on successful login, **Touch** on **`F3 03`** / move-map **`F3 03`**, **Revoke** on TCP disconnect; one active ticket per account (re-login replaces).
- [x] **`TAKUMI_SESSION_TICKET_TTL_MINUTES`** (default **120**, clamp 5..10080) — TTL for ticket validity (M6 game listener will call `TryValidate`).
- [x] Unit tests for ticket store (`SessionTicketStoreTests`).
- [x] **Postgres `session_ticket`** — `sql/init/003_session_ticket.sql` + **`PostgresSessionHandoffRepository`** (`TakumiPostgresMirror.InitSessionHandoffIfEnabled`, env **`TAKUMI_SESSION_HANDOFF_DB`**). Optional strict game login: **`TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF`** (see **`docs/IMPLEMENTATION-CHECKLIST.md`** High Priority). In-memory **`InMemorySessionTicketStore`** unchanged for TTL/Touch on login TCP.
- [x] **Split processes (M5):** `Takumi.Server.LoginHost` (login/game TCP, `LoginTcpOnly`) + `Takumi.Server.ConnectHost` (F4 only). Shared logic: `Takumi.Server.LegacyLoginHost.Runner` + `ConnectServerHostRunner`. Docker profile **`splitstack`**; host scripts `./scripts/run-login-host.sh` + `./scripts/run-connect-host.sh`. Combined single process remains `Takumi.Server.LegacyLoginHost`.
- [ ] **Signed ticket on wire** — implemented (see M6 checklist); optional hardening only.

## Client reference

- **`ReceiveServerConnect`**: `CreateSocket(IP, Data->Port)` — port comes from **F4 03** payload (`ConnectServerInfo602` bytes 20–21 LE).
- **`ReceiveJoinServer`**: **`C1 F1 00`** result `0x01` + `HeroKey` from `NumberH/L` + version bytes — still sent immediately on game/login TCP accept by `LegacyLoginHost` (unchanged).

## Env (see also `env.defaults` / `.env.lan.example`)

| Variable | Role |
|----------|------|
| `TAKUMI_GAME_PORT` | Port in **F4 03** when connect server answers server info. |
| `TAKUMI_SESSION_TICKET_TTL_MINUTES` | In-memory ticket TTL (minutes); also used when extending expiry after join/move-map on login TCP. |
| `TAKUMI_SESSION_HANDOFF_DB` | `1` / `true` = persist pending handoff rows to **`session_ticket`** after login on `LegacyLoginHost` (same PG connection env as roster). |
| `TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF` | On **GameHost** only: `1` = F1 01 success requires a consumable `session_ticket` row (login legacy first). |
| `TAKUMI_GAME_HANDOFF_MATCH_IP` | Default match stored `client_ip`; set `0` to match account only (weaker). |
| `TAKUMI_SESSION_TICKET_HMAC_KEY` | UTF-8 secret (≥8 bytes), same on Legacy + GameHost, for **`F1 A5`/`F1 A6`** signed ticket (see **`docs/M6-GAME-TCP-CHECKLIST.md`**). |
| `TAKUMI_GAME_TICKET_WIRE` | On **GameHost**: `1` = require **`F1 A6`** before **`F1 01`**; consumes row on attach (not IP-only consume). |

## Operational note

If **`TAKUMI_GAME_PORT` ≠ `TAKUMI_LOGIN_PORT`**, you must run **`Takumi.Server.GameHost`** on the game port (or the client will fail to connect after F4 03). Docker publish mapping must expose both ports.
