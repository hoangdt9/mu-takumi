# M9 — Monster viewport + combat stub

**Đã merge trên `main`:** `C2 0x13` spawn, walk rescan, regen, `C1 0x11`/`0x19` hit, `0x16` die, `0x14` destroy on leave.

## Chuẩn bị

- [ ] Docker Up, vào world Lorencia (map 0) hoặc map có spawn.
- [ ] Log: `docker compose logs -f legacy-login 2>&1 | grep '\[m9\]'`
- [ ] (Tuỳ chọn) `adb logcat` — **[android-logcat.md](./android-logcat.md)**

## Bước 1 — Viewport spawn sau join

1. [ ] Login → chọn nhân vật → vào map.
2. [ ] **Host:** `[m9] sent C2 0x13 monster viewport count=N` (N > 0 hoặc fallback Lorencia).
3. [ ] **Client logcat:** `ReceiveCreateMonsterViewport` / `0x13`.

## Bước 2 — Di chuyển (incremental viewport)

1. [ ] Đi bộ ≥ 4 tile (Manhattan) — default `TAKUMI_MONSTER_VIEWPORT_MOVE_TILES=4`.
2. [ ] **Host:** log rescan / thêm destroy+spawn nếu vào vùng mob mới.
3. [ ] Quái mới xuất hiện khi đi vào tầm nhìn.

## Bước 3 — Combat (tap attack)

1. [ ] Đứng trong **≤ 3 tile** (mặc định `TAKUMI_COMBAT_MELEE_RANGE=3`) quái gần nhất.
2. [ ] Tap đánh (client gửi `C1 0x11` hoặc skill `0x19`).
3. [ ] **Host:** `[m9] combat hit` … `died=True/False`.
4. [ ] **Client:** số damage; quái biến mất khi chết.
5. [ ] **Host:** `C1 0x16` die + `C1 0x14` destroy.

## Bước 4 — Regen

1. [ ] Chờ `RegenTime` từ `Monster.txt` (hoặc default catalog).
2. [ ] Đi ra khỏi tầm nhìn rồi quay lại.
3. [ ] **Host:** spawn `0x13` lại cho mob regen.

## Bước 5 — Destroy on leave view

1. [ ] Đứng gần quái → thấy mob.
2. [ ] Đi xa > view range (15 tile mặc định).
3. [ ] **Host:** `[m9] sent C1 0x14 destroy viewport`.
4. [ ] Client: mob biến mất; quay lại → `0x13` spawn lại.

## Env tuning (debug)

| Biến | Mặc định | Ghi chú |
|------|----------|---------|
| `TAKUMI_MONSTER_VIEW_RANGE` | 15 | Manhattan |
| `TAKUMI_COMBAT_STUB_DAMAGE` | 50 | Fallback nếu Defense calc thấp |
| `TAKUMI_MONSTER_SET_BASE_PATH` | auto | Trỏ file thật để nhiều mob hơn fallback |

## Pass criteria

| Bước | Pass |
|------|------|
| 1 | `0x13` sau join |
| 3 | Hit → damage → die/destroy |
| 4 | Regen + respawn in view |
| 5 | Destroy khi ra khỏi view |
