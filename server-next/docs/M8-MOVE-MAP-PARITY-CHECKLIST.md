# M8 — Move map parity (`CMove::Move` + `0x8E`)

**Quy ước:** `[x]` = đã có trong `server-next` và smoke được; `[~]` = một phần; `[ ]` = chưa.

### `MuServer` trong doc này là gì? (không chạy GS C++)

| Thành phần | Vai trò |
|------------|---------|
| **`server-next`** | Runtime thật: `game-host` / `legacy-login` (.NET trong Docker). |
| **`MuServer/4.GameServer/Data`** | **Chỉ file data tĩnh** (`Move.txt`, `Gate.txt`, `CustomArena.txt`, …) — cùng format GS takumi cũ. |
| **Docker mount** | `TAKUMI_GAMESERVER_DATA_HOST` (host) → `TAKUMI_GAMESERVER_DATA_PATH=/muserver-data` (container). Loaders C# đọc từ đây. |

Có thể trỏ `TAKUMI_GAMESERVER_DATA_HOST` sang bản copy khác (vd. `docker/data-zip/host/Data` nếu đủ `Move/`, `Custom/`). **Không** cần build hay chạy binary `4.GameServer` để test M8.

**Smoke M8:** `./scripts/smoke-m8.sh` (catalog logs + mount + 22 unit tests). Nhanh: `./scripts/smoke-m8-move-catalog.sh`.

**Tham chiếu legacy**

| Vai trò | Đường dẫn |
|--------|-----------|
| Logic warp | `Source/4.GameServer/GameServer/Move.cpp` — `CMove::Move`, `CGTeleportMoveRecv` |
| Data | `MuServer/4.GameServer/Data/Move/Move.txt`, `Move/Gate.txt` |
| Client wire | `Source/5.Main/source/WSclient.cpp` — `ReceiveMoveMapChecksum` (`8E 01`), `ReceiveRequestMoveMap` (`8E 03`) |
| Client UI | `NewUIMoveCommandWindow.cpp`, `MoveCommandData.cpp` (file `.bmd` / script local) |
| Key | `KeyGenerater.cpp` — `GenerateKeyValue` / `CheckKeyValue` |

**Ghi chú quan trọng về source cũ**

- **Có sẵn trong repo:** `Move/Move.txt` + `Move/Gate.txt` dưới `MuServer/4.GameServer/Data/` (và bản `Sub 1/`). Docker đã mount qua `TAKUMI_GAMESERVER_DATA_HOST` → `TAKUMI_GAMESERVER_DATA_PATH` (`/muserver-data`).
- **GS takumi (`4.GameServer`) không gửi `GC 0x8E 0x01`** và **không kiểm tra** `DWORD` block key trong `PMSG_TELEPORT_MOVE_RECV::reserved` — chỉ gọi `Move(lpMsg->number)`.
- Client vẫn có handler `8E 01`; nếu không nhận seed, key khởi tạo = 0 và `GetMoveCommandKey()` vẫn sinh giá trị deterministic từ `KeyGenerater`.
- `server-next` log `[m8] MoveMapCatalog: N moves from …` là **mới** (loader C#), không phải log của GS C++ cũ.

---

## P0 — Data & Docker

| # | Hạng mục | Trạng thái | Ghi chú |
|---|----------|------------|---------|
| P0.1 | `Move/Move.txt` trên disk | [x] | `MoveMapCatalog` + `MoveLoader` |
| P0.2 | `Move/Gate.txt` | [x] | `MapGateCatalog` / `MapGateService` |
| P0.3 | `TAKUMI_GAMESERVER_DATA_PATH` | [x] | `docker-compose.yml`, `.env.lan.example` |
| P0.4 | Override `TAKUMI_MOVE_PATH` → file trực tiếp | [x] | |
| P0.5 | ETL `move_map` / `map_gate` Postgres (tùy chọn) | [x] | `import-world-static-data.sh` |
| P0.6 | Startup log số dòng Move | [x] | `[m8] MoveMapCatalog: N moves from …` |
| P0.7 | Smoke: stack recreate, log N > 0 | [x] | `./scripts/smoke-m8.sh` (full) hoặc `smoke-m8-move-catalog.sh`; gọi từ `docker-stack-lan-gamehost.sh` |

---

## P1 — Wire `0x8E`

| # | Packet | Hướng | Trạng thái | Ghi chú |
|---|--------|-------|------------|---------|
| P1.1 | `C1 08 8E 01` + `DWORD` seed | S→C | [x] | `MoveMapOutbound` sau join; log `[m8] move map checksum seed=…` |
| P1.2 | `C1 0A 8E 02` + key + `WORD` index | C→S | [x] | `GamePacketFinders.TryFindMoveMapRequest` |
| P1.3 | `C1 05 8E 03` + `btResult` | S→C | [x] | Success / fail / zen / level |
| P1.4 | Validate block key (`KeyGenerater`) | C→S | [x] | `MoveMapKeyGenerator` (DWORD math); `TAKUMI_MOVE_MAP_SKIP_KEY_CHECK=1` = tắt |
| P1.5 | Gửi lại `8E 01` sau warp OK | S→C | [x] | `MoveMapOutbound.TrySendChecksumAfterJoinAsync` sau warp (trừ `TAKUMI_MOVE_MAP_SKIP_KEY_CHECK=1`) |
| P1.6 | Mã kết quả đầy đủ (`MAPMOVE_FAILED_*`) | S→C | [x] | `MoveMapWire602` 0x00–0x0B + `ToWireResult`; client `ReceiveRequestMoveMap` hiển thị `GlobalText` |

---

## P2 — `Move.txt` / `CMove::GetInfo`

| # | Trường `MOVE_INFO` | Load | Enforce trong `MoveMapService` |
|---|-------------------|------|-------------------------------|
| P2.1 | Index | [x] | [x] |
| P2.2 | Money (zen) | [x] | [x] trừ zen + roster |
| P2.3 | MinLevel / MaxLevel | [x] | [x] |
| P2.4 | MinReset / MaxReset | [x] | [x] `GameRosterEntry.Reset` + enforce khi `MinReset`/`MaxReset` ≠ -1 |
| P2.5 | AccountLevel | [x] | [x] `GameRosterEntry.AccountLevel` + enforce khi `AccountLevel` > 0 |
| P2.6 | Gate → map/xy | [x] | [x] `TryResolveWarpGate` |

---

## P3 — Điều kiện `CMove::Move` (136–224)

| # | Rule | Trạng thái | Ghi chú |
|---|------|------------|---------|
| P3.1 | Unknown move index | [x] | |
| P3.2 | Level min/max (+ gate move level) | [x] | Gate level trong `MapGateService` |
| P3.3 | MinReset / MaxReset | [x] | `MoveMapService` |
| P3.4 | AccountLevel | [x] | `MoveMapService` |
| P3.5 | Zen | [x] | |
| P3.6 | PK ≥ 5 (`m_PKLimitFree`) | [x] | `PlayerPresenceAppearance.PkLevel`; `TAKUMI_PK_LIMIT_FREE=1` bỏ qua |
| P3.7 | Block: interface / teleport / die regen / pshop open | [x] | `PlayerUiSession` (NPC shop/warehouse/trade/pshop + generic interface ref); teleport/dead như cũ |
| P3.8 | Atlans: cấm Uniria/Dinorant (inv slot 8, item 13,2 / 13,3) | [x] | `MoveMapEquipRules` wear slot 8 |
| P3.9 | Icarus / Kanturu3: cần wings slot 7 hoặc Dinorant/Fenrir slot 8 | [x] | `MoveMapEquipRules` maps 10 / 39 |
| P3.10 | Gens: `GENS_FAMILY_NONE` + map Gens battle | [x] | `MapManagerCatalog` + `GameRosterEntry.GensFamily`; wire `0x0A` |
| P3.11 | Custom arena `CA_MAP_RANGE` + `CheckEnterEnabled` | [x] | `CustomArenaScheduleFsm` + section 0 loader; bật FSM: `TAKUMI_CUSTOM_ARENA_SKIP_SCHEDULE=0` + `CustomArenaScheduleLoop` |
| P3.12 | `gObjMoveGate` + `GCMoneySend` | [x] | Teleport + `ShopCommerceWire602.BuildRepair` (zen) sau warp |
| P3.13 | `PShopRedrawAbs` sau warp | [x] | `MoveMapPostWarp` → `PersonalShopWire602.BuildViewportClear` (`C2 3F 00` count=0) sau move-map / gate teleport |

---

## P4 — Teleport / join sau warp

| # | Hạng mục | Trạng thái |
|---|----------|------------|
| P4.1 | `GC 0x1C` teleport (`TeleportWire602`) | [x] |
| P4.2 | Đổi map → `F3 03` join + `F3 10` inventory | [x] |
| P4.3 | Monster scope reset / respawn (`MapMonsterScopeSender`) | [x] |
| P4.4 | Player presence registry update | [x] |
| P4.5 | Persist map/xy/zen roster (+ DB sync) | [x] | `onRosterSave` sau warp OK (`GamePortMinimalSession` → `SaveRoster`) |

---

## P5 — Client (không đổi server nhưng cần QA)

**Checklist thực hiện:** [M8-MOVE-MAP-P5-CLIENT-QA-CHECKLIST.md](./M8-MOVE-MAP-P5-CLIENT-QA-CHECKLIST.md) (đánh dấu `[x]` / `[!]` / `[-]`, gửi mục G để cập nhật doc).

| # | Hạng mục | Trạng thái |
|---|----------|------------|
| P5.1 | `Data\Local\MoveReq.bmd` (hoặc script) — danh sách UI; server dùng `Move.txt` index | [ ] |
| P5.2 | Mở `INTERFACE_MOVEMAP` — `OpenningProcess` / Android grid | [ ] |
| P5.3 | `SettingCanMoveMap` — ẩn map theo level/zen local (server vẫn là authority) | [ ] |
| P5.4 | Lucky seal / change ring — `IsLuckySeal`, `ChangeRingManager` | [ ] |

---

## P6 — Liên quan khác (ngoài `CMove::Move` trực tiếp)

| # | Hạng mục |
|---|----------|
| P6.1 | Gate NPC / `C1 1C` proximity (`MapGateService` proximity) |
| P6.2 | `CGTeleportRecv` scroll/wing teleport (khác move map UI) |
| P6.3 | `CustomNpcMove` / event warp |
| P6.4 | `MapServerManager` multi-GS (không áp dụng single `server-next`) |

---

## Thứ tự triển khai đề xuất

**Server M8 (P0–P4): xong.** Còn QA/smoke và phần ngoài scope:

1. ~~**P0.7**~~ — `./scripts/smoke-m8-move-catalog.sh`.
2. ~~**P1.1 + P1.4 + P1.5**~~ — seed + key + re-seed sau warp.
3. ~~**P2.4–P2.5 + P3.3–P3.9 + P3.12**~~ — reset/account/PK/UI/equip/zen wire.
4. ~~**P3.10**~~ — Gens battle map (`MapManagerCatalog` + `GensFamily`).
5. ~~**P3.11**~~ — Custom arena schedule FSM (`TAKUMI_CUSTOM_ARENA_SKIP_SCHEDULE=0` để bật).
6. ~~**P1.6**~~ — mã `MAPMOVE_FAILED_*` + client feedback.
7. ~~**P4.5**~~ — `SaveRoster` sau warp OK.
8. ~~**P3.7**~~ — `PlayerUiSession` (shop/warehouse/trade/pshop); generic `Interface.use` gắn packet khi cần.
9. ~~**P3.13**~~ — `PShopRedrawAbs` → `C2 3F 00` sau warp.
10. **P5** — client QA: [M8-MOVE-MAP-P5-CLIENT-QA-CHECKLIST.md](./M8-MOVE-MAP-P5-CLIENT-QA-CHECKLIST.md) → confirm mục G.
11. **P6** — gate proximity / scroll teleport / `CustomNpcMove` (milestone khác).
12. ~~**Smoke Docker**~~ — `./scripts/smoke-m8.sh` PASS (43 moves, 523 gates, CustomArena 8+8 schedule rows, 22 unit tests). Còn **in-game**: warp, `SKIP_SCHEDULE=0`, pshop block (P5).

---

## Env nhanh

```bash
# Static data mount (NOT the C++ GameServer process — only txt/bmd on disk)
export TAKUMI_GAMESERVER_DATA_HOST="../MuServer/4.GameServer/Data"
export TAKUMI_GAMESERVER_DATA_PATH="/muserver-data"

# Smoke after stack up (profile gamehost):
# ./scripts/smoke-m8.sh
# ./scripts/smoke-m8.sh --no-recreate   # skip rebuild if game-host already fresh

# Tùy chọn: file Move trực tiếp
# export TAKUMI_MOVE_PATH="/muserver-data/Move/Move.txt"

# Tắt validate key (parity GS C++ cũ — không check)
# export TAKUMI_MOVE_MAP_SKIP_KEY_CHECK=1

# PK murderer block (mặc định bật khi PK >= 5); =1 giống legacy m_PKLimitFree
# export TAKUMI_PK_LIMIT_FREE=1

# Custom arena: mặc định bỏ qua schedule FSM (chỉ check rule/gate/level từ CustomArena.txt)
# export TAKUMI_CUSTOM_ARENA_SKIP_SCHEDULE=0

# MapManager / CustomArena path trực tiếp (tùy chọn)
# export TAKUMI_MAP_MANAGER_PATH="/muserver-data/MapManager.txt"
# export TAKUMI_CUSTOM_ARENA_PATH="/muserver-data/Custom/CustomArena.txt"
```

**Liên kết:** `docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`, `docs/M1-PROTOCOL-PARITY-MAP.md` § `0x8E`.
