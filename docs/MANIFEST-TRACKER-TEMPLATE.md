# Mẫu tracker — manifest §7 & §17 (Takumi → OpenMU)

Dùng cho spreadsheet, GitHub **Projects**, hoặc Issues — **không** thay manifest `.txt`; chỉ là lớp trạng thái động.

## Cột tối thiểu — `TAKUMI-SERVER-SOURCE-MANIFEST.txt` (mỗi dòng một `*.cpp`)

| Cột | Mô tả |
|-----|--------|
| `path` | Ví dụ `Source/4.GameServer/GameServer/Protocol.cpp` |
| `tier` | `P0` (login/network) … `P3` (event ít dùng) |
| `openmu_surrogate` | Tên khối trong fork: `Network`, `GameLogic`, `Plugin.X`, `N/A investigation` |
| `parity_status` | `todo` \| `stub` \| `matches_baseline` \| `wontfix` \| `deferred` |
| `evidence` | Link issue, pcap id, screenshot test, hoặc trống |
| `notes` | Một dòng |

**Gộp chủ đích:** có thể gán cùng `parity_status` cho cả nhóm (vd. mọi `Protect.cpp`, `MiniDump.cpp` server → `wontfix` / thay Observability).

## Cột tối thiểu — `TAKUMI-MUSERVER-GAMEDATA-FILES.txt`

| Cột | Mô tả |
|-----|--------|
| `path_relative` | Ví dụ `Monster/MonsterSetBase.txt` |
| `data_domain` | Một trong: `monster`, `item`, `skill`, `event`, `shop`, … (hoặc `Custom`) |
| `import_path` | `converter_script` \| `manual` \| `openmu_native` \| `not_used` \| `defer` |
| `owner` | Tên nick / nhóm |

## Gate khách quan §17

Khi **`COMPATIBILITY-MATRIX.md`** có scenario **`pass`** với client Takumi trên staging OpenMU, cập nhật dòng **`manifest`-liên quan** + link PR fork.

**Liên kết:** [`TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) §17.
