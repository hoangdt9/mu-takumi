# QA milestone (Takumi + `server-next`)

**Milestone riêng — làm sau dev.** Checklist dev: `server-next/docs/` (dev only).

**Dev verify (mỗi PR):** smoke + unit — vd. `./scripts/smoke/smoke-m8.sh --no-recreate`. **Không** thay QA APK.

**Quy ước:** `[ ]` chưa · `[x]` PASS · `[!]` FAIL · `[-]` SKIP

---

## Index

| Area | QA doc | Dev checklist |
|------|--------|---------------|
| S2 — login / join / world | [qa/S2-gate-login-join.md](./qa/S2-gate-login-join.md) | `server-next/docs/character/M6-GAME-TCP-CHECKLIST.md` |
| M8 — move-map + skill TP | [qa/M8-move-map.md](./qa/M8-move-map.md) | `server-next/docs/world/M8-MOVE-MAP-PARITY-CHECKLIST.md` |
| M9 — monster combat | [qa/M9-monster-combat.md](./qa/M9-monster-combat.md) | `server-next/docs/combat/M9-NPC-MONSTER-CHECKLIST.md` |
| M9 — **MG skill combat (mobile)** | [qa/M9-mg-skill-combat.md](./qa/M9-mg-skill-combat.md) | [MOBILE-SKILL-COMBAT-GUIDE.md](./android/MOBILE-SKILL-COMBAT-GUIDE.md) · [SKILL-COMBAT-ROLLOUT-PLAN.md](./android/SKILL-COMBAT-ROLLOUT-PLAN.md) |
| M9 — NPC shop / gate | [qa/M9-npc-shop.md](./qa/M9-npc-shop.md) | `server-next/docs/combat/M9-M8-NPC-GAMEPLAY-OWNERSHIP.md` |
| Nhật ký QA / dev | [DEVELOPMENT-LOG-2026-05-20.md](./journal/DEVELOPMENT-LOG-2026-05-20.md), [16](./journal/DEVELOPMENT-LOG-2026-05-16.md), [17](./journal/DEVELOPMENT-LOG-2026-05-17.md), [SESSION-WORKLOG-2026-05-19](./journal/SESSION-WORKLOG-2026-05-19.md) | — |
| Android LAN | [ANDROID-DEV-MAC.md](./android/ANDROID-DEV-MAC.md) | `server-next/docs/docker/DOCKER-BUILD-RUN.md` |

---

## Khi nào chạy

- Trước release APK / demo LAN.
- Sau đổi lớn client hoặc wire `0x8E` / `0x1C` / `0x13`.
- **Không** chặn PR dev nhỏ.

---

## Server prep

```bash
cd server-next
./scripts/docker/docker-stack.sh --detach
# .env: TAKUMI_PUBLIC_HOST = LAN IP Mac
```

Smoke (không APK): `./scripts/smoke/smoke-connect-from-host.sh` · `./scripts/smoke/smoke-m8.sh --no-recreate`
