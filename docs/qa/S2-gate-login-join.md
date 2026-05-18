# QA — Gate S2 (login / join / world)

Moved from `server-next/docs/S2-GATE-QA.md`. **Dev:** `server-next/docs/M6-GAME-TCP-CHECKLIST.md`.

Last updated: 2026-05-18

**Gate S2** (`server-next/docs/MONOGAME-CLIENT-ROADMAP.md`): game TCP + login + join + inventory ổn định.

---

## 1. Server prep

```bash
cd server-next
./scripts/docker-stack.sh --detach
```

`.env`: `TAKUMI_PUBLIC_HOST`, `TAKUMI_GAME_PORT=55901`, `TAKUMI_ROSTER_DB_SYNC=1`. DB user compose: **`takumi`** (không `postgres`).

Reset roster (tuỳ chọn):

```bash
./scripts/reset-roster-account.sh test
docker compose restart game-host legacy-login
```

---

## 2. Log expectations

| Step | Log |
|------|-----|
| Connect | `ServerList`, `ServerInfo port=55901` |
| Login | `login ok id=test` (không Npgsql `127.0.0.1:54444` trong container) |
| Join | `F3 00` / character list |

---

## 3. Android logcat

`ReceiveJoinServer result=0x01`, `Translate F1 sub=0x01 value=0x01`, `LoadWorld` không kẹt màn hình đen.

---

## 4. Smoke Mac (no phone)

```bash
./scripts/smoke-connect-from-host.sh 127.0.0.1 44605
```

---

## 5. Sign-off

- [ ] Docker healthy
- [ ] Login `test`/`test`
- [ ] Char list + enter world
