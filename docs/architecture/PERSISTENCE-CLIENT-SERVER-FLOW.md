# Luồng chỉ số / skill / DB ↔ client (Takumi server-next)

Tài liệu mô tả **toàn bộ nhân vật** (DW/DK/ELF/MG/DL/SU/RF theo `CharacterSheetCalculator.ClassIndex = serverClass / 0x20`), không chỉ MG.

## Bật Postgres mirror

- Biến môi trường: connection string / `TAKUMI_PG_*`, và `TAKUMI_ROSTER_DB_SYNC=1` (xem `TakumiPostgresMirror.InitIfEnabled`).
- Áp SQL trong `server-next/sql/init/` + `patches/` (đặc biệt `009_character_stats.sql`, `016_character_skill.sql`, `013_character_key_configuration.sql`).
- Nếu thiếu bảng (`character_skill`), repo log cảnh báo — **skill học từ orb chỉ còn trong phiên**, không relog.

## Dữ liệu persistent (Postgres)

| Khía cạnh | Bảng / nơi lưu | Ghi chú |
|-----------|----------------|---------|
| Vị trí map, level, EXP, zen, HP/MP/SD hiện tại & max (join), **STR/DEX/VIT/ENE/Lead**, **level_up_point**, BP | `character_roster` | `PostgresCharacterRosterRepository` + `UpsertProgressAsync` từ `CharacterRosterMirrorWriter` |
| Bản sao “domain” (mirror progress) | `character_domain` | Cùng progress/stats khi domain sync bật (`CharacterDomainMirrorWriter`) |
| Ô túi | `inventory_slot` | `JoinInventoryLifecycle`, `PlayerShopSession` |
| Skill đã học (slot/type/level) | `character_skill` | `JoinSkillLifecycle`, `ItemWorldHandler` (orb/scroll) |
| Warehouse, zen tài khoản | `warehouse_slot`, wallet tables | Độc lập nhân vật |
| Hotkey / QWER blob | `character_roster.key_configuration` | `C1 F3 30` ↔ `OptionDataWire602` |

**Lưu ý EXP trong RAM:** `GameRosterEntry.Experience` là `uint`; load từ DB `long` được clamp `uint.MaxValue` (`CharacterRosterEntryMapping`). DB vẫn lưu `BIGINT` đủ lớn cho tích lũy.

## Luồng join game (sau chọn nhân vật)

Cùng một thứ tự trên **`GamePortMinimalSession`** và **`LegacyLoginHostRunner`** (đoạn join map):

1. **`C1 F3 03`** — `JoinMapServerWire602`: map/xy/góc, EXP + next threshold, **level_up_point**, 4 stat cơ bản + vitals, View* DWORDs (gồm leadership), … — dữ liệu từ `GameRosterEntry.ToWireWithSheet()` (sheet mặc định theo class+nếu DB=0).
2. **`F3 10`** — inventory sync (`JoinInventoryLifecycle`).
3. **`F3 11`** — danh skill: load `character_skill`; nếu rỗng thì seed **`CharacterSkillCatalog.GetDefaultEntries(serverClass)`**, ghi lại DB (`JoinSkillLifecycle`). Wire **byte[4]=count, byte[5]=list type** (đúng struct client / OpenMU).
4. **`F3 30`** — option/hotkey (`OptionDataWire602.BuildApply`).
5. Life/mana sync + **`F3 E1`** calc broadcast (tùy `RosterVitalsLifecycle`).

Mọi **`serverClass`** wire hợp lệ đều đi qua cùng đường; MG không đặc biệt ở join.

## Luồng runtime (trong game)

| Hành động | Client → server | Server → client | Mirror DB |
|-----------|------------------|-----------------|-----------|
| Cộng điểm | `C1 F3 06` (5 hoặc 7 byte, có thể XOR) | `F3 06` OK + `F3 E1` | `RosterProgressMirror` → `UpsertProgressAsync` (stats + vitals + level_up_point) |
| Nhận EXP (kill) | (combat) | (damage/death) | `RosterExperienceCombat` → progress upsert (level, EXP, điểm dư, vitals) |
| Học skill orb/scroll | Item use | `F3 11` với `Count=0xFE` + delete item | `character_skill.ReplaceAll` (+ inventory slot) |
| Lưu hotkey | `F3 30` | Echo / apply | Key blob trên roster row khi handler lưu (xem `CharacterKeyConfiguration`) |

**Leadership (stat type 4):** chỉ **Dark Lord** (`ClassIndex == 4`) mới áp dụng qua `TryAddStatPoints(..., serverClass, ...)`; các class khác client gửi type 4 → server **không** trừ điểm / không cộng chỉ số (fail toàn batch nếu kèm request hợp lệ khác—tùy payload; thường client PC không gửi Leadership cho non-DL).

## Skill mặc định theo class

`CharacterSkillCatalog` switch theo `ClassIndex`: 0 DW, 1 DK, 2 FE, 3 MG, 4 DL, 5 SU, 6 RF, default DW. Mỗi class có mảng skill type riêng — **không hard-code MG**.

Orb/scroll học thêm skill: `InventorySkillOrbRules` / `InventoryEtcSkillScrollRules` dùng **class mask** đa class (bit theo `ClassIndex`), không giới hạn MG.

## Kiểm tra nhanh sau thay đổi

```bash
dotnet test server-next/src/Takumi.Server.Tests/Takumi.Server.Tests.csproj \
  --filter "FullyQualifiedName~MagicList|FullyQualifiedName~CharacterSkill|FullyQualifiedName~CharacterStat"
```

## Góc còn thiếu / ngoài phạm vi hiện tại

- Master skill tree / Season 6+ master packets nếu client bật master mode.
- Đồng bộ zen túi với `inventory_slot` (roster `zen` và túi có thể diverge tùy handler).
- Skill teleport / map rules riêng một số skill (ví dụ `SkillTeleportService` có nhánh MG) — gameplay, không phải persist stats chung.
