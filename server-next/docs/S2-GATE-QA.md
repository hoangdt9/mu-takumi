# Gate S2 — QA checklist (server + C++ client)

Last updated: 2026-05-16

**Gate S2** (see `MONOGAME-CLIENT-ROADMAP.md`): game TCP + login + join + inventory stable enough to start MonoGame Phase 1–2.

**Blockers fixed in tree (recreate Docker after pull):**

- `DockerRuntimeEnv` — Postgres `postgres:5432` inside containers (not `127.0.0.1:54444` from `.env`).
- Split-port Android: game-host clears `TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF` unless `TAKUMI_DOCKER_GAMEHOST_REQUIRE_HANDOFF=1`.
- `GamePortMinimalSession` — handoff DB errors return login fail instead of disconnect.

---

## 1. Server prep

```bash
cd server-next
./scripts/docker-stack.sh --detach
# đợi [legacy-login] build OK và [game-host] listening on *:55901
```

`.env` minimum:

- `TAKUMI_PUBLIC_HOST` = LAN IP Mac
- `TAKUMI_GAME_PORT=55901` (if using gamehost profile)
- `TAKUMI_ROSTER_DB_SYNC=1` + Postgres (Docker sets host automatically)

Apply SQL on old volumes if needed:

```bash
./scripts/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime"
```

---

## 2. Host log expectations (one login attempt)

| Step | legacy-login / game-host |
|------|---------------------------|
| Connect | `sent … ServerList`, `ServerInfo ip=… port=55901` |
| Game TCP | `sent join C1 F1 00` |
| Login | `login ok id=test rosterCount=…` (**not** Npgsql to 127.0.0.1) |
| After login | `after login (auto)` or `F3 00` character list bytes |

**Fail patterns:**

- `Failed to connect to 127.0.0.1:54444` → recreate containers (Docker env override).
- `login rejected: no consumable session_ticket` → handoff on without legacy login; use Docker defaults or set `TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF=0` on game-host.

---

## 3. Android / C++ logcat (`TakumiErrorReport`)

| Step | Log |
|------|-----|
| C2 list | `Success Receive Server List` |
| F4 03 | `ReceiveServerConnect`, `port = 55901` |
| Join | `ReceiveJoinServer result=0x01` |
| Login TX | `send packet … head=0x19` (encrypted login) |
| Login RX | `Translate F1 sub=0x01 value=0x01` |
| Char list | `F3 00` / character slots visible |
| Select + join | `F3 03` / `LoadWorld` without black screen |

```bash
adb logcat -c
adb logcat -v threadtime TakumiErrorReport:I MuPreload:I '*:S'
```

---

## 4. Smoke from Mac (no phone)

```bash
./scripts/smoke-connect-from-host.sh 127.0.0.1 44605
./scripts/check-lan-connect-ports.sh
```

---

## 5. Gate S2 sign-off

- [ ] Docker `game-host` + `legacy-login` healthy after `build OK`
- [ ] Login `test`/`test` → `login ok` in game-host log
- [ ] Android: `Translate F1 … value=0x01` after login packet
- [ ] Character list non-empty or create-char works
- [ ] Enter world (no hang on black screen after select)

When all checked → update `MONOGAME-CLIENT-ROADMAP.md` gate table (**S2 đạt**) and start `client-mono` Phase 1.
