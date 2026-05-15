# M10 — Map presence broadcast

**Đã merge (`b882608`):** `GameMapPresenceRegistry`, `PlayerPositionWire602` (`0x15`), `PlayerActionWire602` (`0x18`) trên join và map move.

**Anti-flood:** `TAKUMI_PRESENCE_MAX_BROADCASTS_PER_SECOND` giới hạn outbound `0x15`/`0x18` broadcast (0 = tắt).

**Chưa đầy đủ:** miss/skill % combat (WIP mac-m4).

## Chuẩn bị

- [ ] Docker Up; `TAKUMI_VERBOSE=1` hoặc `TAKUMI_STRUCTURED_LOG=1`.
- [ ] Filter log:

```bash
docker compose logs -f legacy-login 2>&1 | grep -E '\[m10\]|presence|0x15|0x18|GameMapPresence'
```

## Bước 1 — Join map (self presence)

1. [ ] Login → vào map.
2. [ ] **Host:** đăng ký presence sau `F3 03` + monster viewport (không exception).
3. [ ] Client: nhân vật đứng đúng tile roster.

## Bước 2 — Walk / instant move

1. [ ] Đi bộ hoặc blink (nếu client gửi move packet).
2. [ ] **Host:** cập nhật tile trong registry; có thể gửi position wire cho session.
3. [ ] Disconnect → reconnect: vị trí lưu M4/M7 roster vẫn đúng.

## Bước 3 — Map change (nếu có warp QA)

1. [ ] Chuyển map (dev warp / gate khi có).
2. [ ] **Host:** `GameMapPresenceRegistry` unregister map cũ, register map mới.
3. [ ] Không crash; monster viewport M9 chạy lại trên map mới.

## Bước 4 — Hai APK (full broadcast)

1. [ ] Hai thiết bị, hai account, **cùng map** (Lorencia spawn gần nhau).
2. [ ] Account A đi bộ; account B **nhìn thấy** nhân vật A di chuyển (không chỉ self).
3. [ ] **Host A hoặc B:** `[m10] broadcast C1 0x15 … peers=1`
4. [ ] (Tuỳ chọn) set `TAKUMI_PRESENCE_MAX_BROADCASTS_PER_SECOND=20` — spam walk không flood disconnect.

```bash
# .env
TAKUMI_MAP_PRESENCE_ENABLED=1
TAKUMI_PRESENCE_MAX_BROADCASTS_PER_SECOND=60
TAKUMI_VERBOSE=1
```

## M6 game-host path

Nếu client dùng **55901** sau F4 03:

- [ ] `docker compose ps` có `takumi-game-host`.
- [ ] Lặp bước 1–2 trên log `game-host` thay vì chỉ `legacy-login`.

## Pass criteria (minimal)

- [ ] Join/move không lỗi; log presence/register.
- [ ] Roster tile cập nhật sau walk (M4c + M10).
- [ ] (Stretch) hai client cùng map thấy nhau — future hardening.
