# QA — M9 monster viewport + combat (APK)

**Dev:** `server-next/docs/M9-NPC-MONSTER-CHECKLIST.md`, `docs/M9-MONSTER-AI-PORT-CHECKLIST.md` · log `[m9]`

**Quy ước:** `[ ]` · `[x]` · `[!]` · `[-]`

---

## Chuẩn bị

- [ ] Docker up, vào world (Lorencia map 0)
- [ ] `docker compose logs -f game-host 2>&1 | grep '\[m9\]'`
- [ ] (Tuỳ chọn) `adb logcat` — filter `0x13`, `ReceiveCreateMonsterViewport`

---

## Viewport sau join

- [ ] Login → chọn char → vào map
- [ ] Host: `[m9] sent C2 0x13 monster viewport count=N` (N > 0)
- [ ] Client: nhận monster viewport

---

## Walk rescan

- [ ] Đi ≥ 4 tile — mob mới/destroy khi vào/ra tầm nhìn

---

## Combat

- [ ] Tap đánh mob ≤ 3 tile — host `[m9] combat hit`
- [ ] Mob chết: `0x16` + `0x14`
- [ ] Player mất HP khi đứng gần mob aggro

---

## Regen + leave view

- [ ] Chờ regen / đi xa rồi quay lại — `0x13` lại
- [ ] Đi xa > view range — `0x14` destroy

Ghi chú: ____
