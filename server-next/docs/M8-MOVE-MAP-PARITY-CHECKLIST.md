# M8 — Move map parity (`CMove::Move` + `0x8E`)

**Quy ước:** `[x]` = đã có trong `server-next` và smoke được; `[~]` = một phần; `[ ]` = chưa.

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
| P0.7 | Smoke: stack recreate, log N > 0 | [ ] | `docker-stack.sh --recreate` |

---

## P1 — Wire `0x8E`

| # | Packet | Hướng | Trạng thái | Ghi chú |
|---|--------|-------|------------|---------|
| P1.1 | `C1 08 8E 01` + `DWORD` seed | S→C | [x] | `MoveMapOutbound` sau join; log `[m8] move map checksum seed=…` |
| P1.2 | `C1 0A 8E 02` + key + `WORD` index | C→S | [x] | `GamePacketFinders.TryFindMoveMapRequest` |
| P1.3 | `C1 05 8E 03` + `btResult` | S→C | [x] | Success / fail / zen / level |
| P1.4 | Validate block key (`KeyGenerater`) | C→S | [x] | `MoveMapKeyGenerator` (DWORD math); `TAKUMI_MOVE_MAP_SKIP_KEY_CHECK=1` = tắt |
| P1.5 | Gửi lại `8E 01` sau warp OK | S→C | [ ] | Một số GS gửi lại seed; client chỉ cần seed ban đầu nếu advance đúng |
| P1.6 | Mã kết quả đầy đủ (`MAPMOVE_FAILED_*`) | S→C | [~] | Client comment trong `WSclient.cpp`; hiện chủ yếu 0x00/0x01/0x07/0x08 |

---

## P2 — `Move.txt` / `CMove::GetInfo`

| # | Trường `MOVE_INFO` | Load | Enforce trong `MoveMapService` |
|---|-------------------|------|-------------------------------|
| P2.1 | Index | [x] | [x] |
| P2.2 | Money (zen) | [x] | [x] trừ zen + roster |
| P2.3 | MinLevel / MaxLevel | [x] | [x] |
| P2.4 | MinReset / MaxReset | [x] | [ ] cần `Reset` trên roster/DB |
| P2.5 | AccountLevel | [x] | [ ] cần VIP/account level session |
| P2.6 | Gate → map/xy | [x] | [x] `TryResolveWarpGate` |

---

## P3 — Điều kiện `CMove::Move` (136–224)

| # | Rule | Trạng thái | Ghi chú |
|---|------|------------|---------|
| P3.1 | Unknown move index | [x] | |
| P3.2 | Level min/max (+ gate move level) | [x] | Gate level trong `MapGateService` |
| P3.3 | MinReset / MaxReset | [ ] | |
| P3.4 | AccountLevel | [ ] | |
| P3.5 | Zen | [x] | |
| P3.6 | PK ≥ 5 (`m_PKLimitFree`) | [ ] | Cần PK level trên roster |
| P3.7 | Block: interface / teleport / die regen / pshop open | [ ] | Cần flags session/UI |
| P3.8 | Atlans: cấm Uniria/Dinorant (inv slot 8, item 13,2 / 13,3) | [ ] | |
| P3.9 | Icarus / Kanturu3: cần wings slot 7 hoặc Dinorant/Fenrir slot 8 | [ ] | |
| P3.10 | Gens: `GENS_FAMILY_NONE` + map Gens battle | [ ] | |
| P3.11 | Custom arena `CA_MAP_RANGE` + `CheckEnterEnabled` | [ ] | |
| P3.12 | `gObjMoveGate` + `GCMoneySend` | [~] | Teleport + zen; broadcast money packet [ ] |
| P3.13 | `PShopRedrawAbs` sau warp | [ ] | |

---

## P4 — Teleport / join sau warp

| # | Hạng mục | Trạng thái |
|---|----------|------------|
| P4.1 | `GC 0x1C` teleport (`TeleportWire602`) | [x] |
| P4.2 | Đổi map → `F3 03` join + `F3 10` inventory | [x] |
| P4.3 | Monster scope reset / respawn (`MapMonsterScopeSender`) | [x] |
| P4.4 | Player presence registry update | [x] |
| P4.5 | Persist map/xy/zen roster (+ DB sync) | [~] |

---

## P5 — Client (không đổi server nhưng cần QA)

| # | Hạng mục |
|---|----------|
| P5.1 | `Data\Local\MoveReq.bmd` (hoặc script) — danh sách UI; server dùng `Move.txt` index |
| P5.2 | Mở `INTERFACE_MOVEMAP` — `OpenningProcess` / Android grid |
| P5.3 | `SettingCanMoveMap` — ẩn map theo level/zen local (server vẫn là authority) |
| P5.4 | Lucky seal / change ring — `IsLuckySeal`, `ChangeRingManager` |

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

1. **P0.7** — xác nhận Docker log `N moves > 0`.
2. **P1.1 + P1.4** — `8E 01` + validate key (bước hiện tại).
3. **P2.4–P2.5 + P3.3–P3.4** — reset / account level khi M7 roster có field.
4. **P3.6–P3.7** — PK + trạng thái UI/shop.
5. **P3.8–P3.10** — equip / Gens (cần inventory snapshot đáng tin).
6. **P1.6 + P3.12** — mã lỗi đầy đủ + `GCMoneySend`.

---

## Env nhanh

```bash
# Data (host)
export TAKUMI_GAMESERVER_DATA_HOST="../MuServer/4.GameServer/Data"
export TAKUMI_GAMESERVER_DATA_PATH="/muserver-data"

# Tùy chọn: file Move trực tiếp
# export TAKUMI_MOVE_PATH="/muserver-data/Move/Move.txt"

# Tắt validate key (parity GS C++ cũ — không check)
# export TAKUMI_MOVE_MAP_SKIP_KEY_CHECK=1
```

**Liên kết:** `docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`, `docs/M1-PROTOCOL-PARITY-MAP.md` § `0x8E`.
