# M8 P5 — Client QA checklist (move-map UI)

**Mục đích:** QA trên **APK/client** sau khi server M8 (P0–P4) đã pass `./scripts/smoke-m8.sh`.  
**Không** cần chạy GS C++ `MuServer` — chỉ cần data mount (`Move.txt`, …) trong Docker.

**Liên kết:** [M8-MOVE-MAP-PARITY-CHECKLIST.md](./M8-MOVE-MAP-PARITY-CHECKLIST.md) (P5), [DOCKER-BUILD-RUN.md](./DOCKER-BUILD-RUN.md)

**Quy ước đánh dấu:** `[ ]` chưa test · `[x]` PASS · `[!]` FAIL · `[-]` SKIP (ghi lý do)

---

## A — Chuẩn bị

| # | Việc | OK |
|---|------|-----|
| A.1 | `cd server-next` — stack up: `docker compose ps` thấy `legacy-login`, `game-host` | [ ] |
| A.2 | Smoke server: `./scripts/smoke-m8.sh --no-recreate` → PASS | [ ] |
| A.3 | `.env`: `TAKUMI_PUBLIC_HOST` = IP Mac (cùng Wi‑Fi với điện thoại) | [ ] |
| A.4 | APK cài xong, login → **vào game** (có nhân vật trên map) | [ ] |
| A.5 | (Tuỳ chọn) Terminal log: `docker compose logs -f game-host` | [ ] |
| A.6 | (Tuỳ chọn) Logcat: `adb logcat \| grep -iE 'move map\|8E\|MoveCommand'` | [ ] |

**Ghi khi confirm:** Ngày ____ · APK/build ____ · Nhân vật (tên/level/zen) ____

---

## B — P5.1 `MoveReq.bmd` vs server `Move.txt` (index)

Client đọc `Data/Local/MoveReq.bmd` (hoặc script); server dùng `Move/Move.txt` **index**.

| # | Bước | Pass khi | Kết quả |
|---|------|----------|---------|
| B.1 | Mở panel move-map (**M** hoặc nút UI mobile) | Panel mở, có danh sách map, không crash | [ ] |
| B.2 | Thấy tên **Lorencia**, **Noria**, **Devias** | Tên đọc được, không `???` | [ ] |
| B.3 | Chọn **Lorencia** → warp | Dịch chuyển OK, không kẹt loading | [ ] |
| B.4 | Log `game-host`: `[m8] move map index=…` | Index khớp map đã bấm (Lorencia thường **2** trên data takumi) | [ ] |
| B.5 | Chọn **Noria** → warp | OK + ghi index log: ____ | [ ] |

**Ghi chú / FAIL:** _______________________________________________

---

## C — P5.2 UI move-map (`INTERFACE_MOVEMAP` / Android grid)

| # | Bước | Pass khi | Kết quả |
|---|------|----------|---------|
| C.1 | Đóng/mở panel 3 lần | Không crash | [ ] |
| C.2 | **Android:** lưới ~**4 cột**, có **phân trang** nếu nhiều map | Tile chạm được | [ ] |
| C.3 | Warp **cùng map / gần** (vd. Lorencia ↔ Noria) | Load bình thường | [ ] |
| C.4 | Warp **đổi map** (vd. Lorencia → Devias) | Teleport + spawn đúng map | [ ] |
| C.5 | Sau warp OK, mở lại move-map | Panel mở được; warp lần 2 vẫn OK | [ ] |
| C.6 | (Tuỳ chọn) Logcat / log: nhận `8E 01` seed sau join/warp | Không lỗi key liên tục | [ ] |

**Ghi chú / FAIL:** _______________________________________________

---

## D — P5.3 `SettingCanMoveMap` (client xám vs server chat)

Client lọc level/zen/PK local; **server là authority** — từ chối phải có **system chat**.

| # | Kịch bản | Cách test | Pass khi | Kết quả |
|---|----------|-----------|----------|---------|
| D.1 | Level thấp | Char level **&lt;** MinLevel map (vd. Tarkan 140) | Map xám **hoặc** chat báo lỗi khi bấm | [ ] |
| D.2 | Thiếu zen | Zen &lt; phí move (`Move.txt` Money) | Xám hoặc chat thiếu tiền | [ ] |
| D.3 | PK murderer | PK ≥ 5 (nếu test được) | Không warp + có chat | [ ] |
| D.4 | NPC shop mở | Talk shop → mở shop → thử move-map | Warp **không** chạy | [ ] |
| D.5 | Personal shop mở | Mở pshop → thử move-map | Warp **không** chạy | [ ] |
| D.6 | Warehouse mở | Mở kho → thử move-map | Warp **không** chạy | [ ] |

**Ghi chú / FAIL:** _______________________________________________

---

## E — P5.4 Lucky seal / Change ring (Icarus / Atlans)

| # | Bước | Pass khi | Kết quả |
|---|------|----------|---------|
| E.1 | Không cánh / không dinorant → thử **Icarus** | Không warp (xám hoặc server/chat) | [ ] |
| E.2 | Có **cánh** (slot 7) hoặc **Dinorant** (13,3) → thử **Icarus** | Cho phép nếu đủ level/zen | [ ] |
| E.3 | **Uniria** (13,2) hoặc Dinorant trên helper → thử **Atlans** | Bị chặn | [ ] |
| E.4 | Change ring cấm Icarus (nếu có item) | Vẫn chặn dù có cánh | [ ] `[-]` SKIP |

**Ghi chú / FAIL:** _______________________________________________

---

## F — Bổ sung server (nên test cùng lúc)

| # | Việc | Pass khi | Kết quả |
|---|------|----------|---------|
| F.1 | Lorencia → Noria → quay lại | Không stuck, xy hợp lý | [ ] |
| F.2 | Sau warp, quan sát shop title người khác (nếu có) | Không ghost UI lạ | [ ] |
| F.3 | (Tuỳ chọn) `TAKUMI_CUSTOM_ARENA_SKIP_SCHEDULE=0` + warp gate arena | Trong/ngoài cửa sổ schedule đúng | [ ] `[-]` SKIP |

**Ghi chú / FAIL:** _______________________________________________

---

## G — Mẫu confirm (copy gửi lại để cập nhật doc)

```text
M8 P5 QA — ngày: YYYY-MM-DD
APK/build: 
Server: smoke-m8 PASS / game-host recreated: yes/no
Nhân vật: name=  level=  zen=  map=

P5.1 MoveReq vs Move.txt:     [x] PASS  [ ] FAIL  [ ] SKIP — 
P5.2 UI / Android grid:       [x] PASS  [ ] FAIL  [ ] SKIP — 
P5.3 SettingCanMoveMap/chat:  [x] PASS  [ ] FAIL  [ ] SKIP — 
P5.4 Lucky seal / ring:       [x] PASS  [ ] FAIL  [x] SKIP — 

F1 round-trip warp:           [x] PASS  [ ] FAIL
F2 pshop viewport after warp: [x] PASS  [ ] FAIL  [ ] SKIP
F3 custom arena schedule:     [ ] PASS  [ ] FAIL  [x] SKIP

Log game-host (paste 1–3 dòng [m8] move map):
Screenshot (optional): yes/no
```

---

## H — Sau khi bạn confirm

Mình sẽ cập nhật `M8-MOVE-MAP-PARITY-CHECKLIST.md`:

- Cột trạng thái **P5.1–P5.4** → `[x]` / `[~]` / `[ ]`
- Bước **10** trong “Thứ tự triển khai” + link file này
- Dòng ngày / build QA (nếu bạn cung cấp)
