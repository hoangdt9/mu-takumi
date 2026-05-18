# QA — M8 move-map (APK)

**Dev:** `server-next/docs/M8-MOVE-MAP-PARITY-CHECKLIST.md` + `./scripts/smoke-m8.sh --no-recreate`

**Quy ước:** `[ ]` · `[x]` · `[!]` · `[-]`

---

## A — Chuẩn bị

| # | Việc | OK |
|---|------|-----|
| A.1 | `docker compose ps` → `legacy-login`, `game-host` | [ ] |
| A.2 | `./scripts/smoke-m8.sh --no-recreate` → PASS | [ ] |
| A.3 | `TAKUMI_PUBLIC_HOST` = IP Mac (cùng Wi‑Fi APK) | [ ] |
| A.4 | APK → login → vào game | [ ] |

---

## B — Index / warp

| # | Bước | Kết quả |
|---|------|---------|
| B.1 | Mở move-map | [ ] |
| B.2 | Lorencia, Noria, Devias hiển thị | [ ] |
| B.3 | Lorencia warp (log index **2**) | [ ] |
| B.4 | Noria warp | [ ] |

---

## C — UI

| # | Bước | Kết quả |
|---|------|---------|
| C.1 | Đóng/mở panel ×3 | [ ] |
| C.2 | Warp đổi map | [ ] |
| C.3 | Warp lần 2 sau warp OK | [ ] |

---

## D — Server authority

| # | Kịch bản | Kết quả |
|---|----------|---------|
| D.1 | Level / zen không đủ | [ ] |
| D.2 | Shop / pshop / warehouse mở | [ ] |

---

## E — Equip (Icarus / Atlans)

| # | Bước | Kết quả |
|---|------|---------|
| E.1 | Không cánh → Icarus chặn | [ ] |
| E.2 | Có cánh → Icarus OK | [ ] |
| E.3 | Helper mount → Atlans chặn | [ ] |

---

## F — Skill teleport (MG, `gate==0`)

| # | Bước | Kết quả |
|---|------|---------|
| F.1 | MG có skill Teleport → click map gần (≤8 ô) | [ ] |
| F.2 | Đích không hợp lệ → không dịch / snap lại | [ ] |
| F.3 | MP giảm sau teleport | [ ] |

## G — Gate NPC (`0x1C`, không phải move-map UI)

| # | Bước | Kết quả |
|---|------|---------|
| G.1 | Đứng cổng Lorencia↔Dungeon (±5 ô) → warp | [ ] |
| G.2 | Xa cổng → không warp | [ ] |

Chi tiết shop/gate: [M9-npc-shop.md](./M9-npc-shop.md)

Ghi chú: ____ · Log `[m8] move map` / `[m8] skill teleport`: ____

