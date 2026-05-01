# Phase 2 — SQL Takumi legacy ↔ OpenMU (PostgreSQL / EF)

**Mục đích:** Bổ sung **migration dữ liệu** sau inventory proc/bảng trong [`TAKUMI-SQL-BACKLOG.md`](TAKUMI-SQL-BACKLOG.md). Repo **OpenMU** tham chiếu: ví dụ `OpenMU/` trên máy dev (`src/DataModel`, `src/Persistence/EntityFramework`).

**Chân lý schema đầy đủ:** vẫn là **`MuServer/7.DataBase/MuOnline.bak`** (+ restore MSSQL); `SQLUp.sql` chỉ là **patch custom** đã áp vào một bản MU cụ thể.

---

## 1. DDL trong `MuServer/7.DataBase/SQL Back/SQLUp.sql` (đầy đủ file)

### 1.1 Cột thêm vào bảng vanilla `[Character]`

| Cột Takumi | Kiểu (script) |
|------------|----------------|
| `DanhHieu` | `int NOT NULL DEFAULT 0` |
| `LucChien` | `int NOT NULL DEFAULT 0` |
| `TimeReset` | `smalldatetime NULL` |
| `mLvVIPChar` | `int NOT NULL DEFAULT 0` |
| `mPointVIPChar` | `int NOT NULL DEFAULT 0` |

### 1.2 Bảng custom mới

| Bảng | Mục đích gợi ý |
|------|----------------|
| **`DataNapGame`** | Nạp tiền / backlog nạp (Account, Name, TienNap, Checking, Status). |
| **`ItemMarketData`** | Chợ / bán đồ kiểu item blob (`varbinary(16)` + giá/khóa); logic Season custom. |
| **`CustomItemBank`** | Ngân hàng jewel/item theo `AccountID` + index/count. |
| **`EquipInventory`** | Kho equip lưu **blob** `Items varbinary(3840)` keyed `CharName` — không chuẩn hoá như EF OpenMU. |

*(Chi tiết cột trong file gốc; không nhân bản DDL để tránh drift.)*

---

## 2. So sánh khái niệm với OpenMU (EF)

Tham chiếu entity chính: **`OpenMU/src/DataModel/Entities/Character.cs`**, **`OpenMU/src/DataModel/Entities/Account.cs`**. Postgres: bảng và schema do **EF migrations** trong `OpenMU/src/Persistence/EntityFramework/Migrations/` và attribute `Table(..., Schema = SchemaNames.*)`. Trên DB thực tế tên schema là **`data`** (hằng C# `SchemaNames.AccountData`), **`config`** (`Configuration`), **`friend`**, **`guild`** — khi dùng `takumi-pg-inspect` truyền `--schema data` / `--schema config`.

### 2.1 Nhóm “character core” vanilla

OpenMU **`Character`** dùng `Guid Id`, **`CharacterClass`** (FK), `Experience`, **`ItemStorage`** (inventory), **`StatAttribute`** collection; **không** tương đương 1:1 một row bảng **`Character`** MSSQL cũ.

- Parse cột/binary inventory & mu-style item encoding từ legacy → **`Item`** / **`ItemSlot`** của OpenMU.
- Map **`CurrentMap`** + tọa độ → **`GameMapDefinition`** (Guid) của world target.

### 2.2 Cột Takumi chỉ trong `SQLUp.sql` (Season / custom)

| Legacy (Takumi) | OpenMU vanilla | Ghi chú Phase 2 |
|-----------------|----------------|-----------------|
| `DanhHieu`, `LucChien` | Không có field tướng đương | **Plug-in Takumi**: bảng phụ Postgres (`character_cosmetic`), hoặc `StatDefinition`/`Attributes` tuỳ thiết kế fork. |
| `TimeReset`, `mLvVIPChar`, `mPointVIPChar` | Không có | Plug-in VIP / reset; hoặc cột shadow trên extension table `character_takumi`. |
| **`DataNapGame`** | Không trong core DataModel | Bảng mới trong plugin persistence. |
| **`ItemMarketData`** | Không thấy entity “player market stall DB” tương đương trong `DataModel` | **Fork**: implement market service hoặc WONTFIX phase 1. |
| **`CustomItemBank`** | Gần với khái niệm kho/account — nhưng OpenMU dùng storage riêng | Map sang **inventory extension** hoặc bảng plugin. |
| **`EquipInventory` (blob)** | OpenMU **`ItemStorage`** chuẩn hoá | Cần **decoder** blob legacy → slots; không dán nguyên `varbinary`. |

### 2.3 Thủ tục `WZ_*` (đã danh trong backlog)

- **Vanilla-flavored:** `WZ_CreateCharacter`, siege `WZ_CS_*`, `WZ_GetItemSerial`, ranking… — xử lý bằng **logic đã có** trong GameServer/OpenMU hoặc import world qua tooling; không giữ ODBC `EXEC` trong runtime mới.
- **Custom Takumi:** `WZ_Custom*`, `WZ_SetReward*`, `WZ_TvTRanking`, … → **PostgreSQL functions** hoặc **C# services** trong plugin; migrate **dữ liệu kết quả ranking** chứ không “port proc” verbatim.

---

## 3. ADR nháp — Chiến lược ETL (chưa chốt reviewer)

**A) Migrate đầy đủ:** `MuOnline.bak` → script ETL từng bảng → Postgres OpenMU-shape. Ưu: giữ nhân vật/warehouse tối đa. Nhược: giải mã blob, map map ID, chỉnh custom cực tốn công.

**B) Fresh world OpenMU:** admin tạo world sạch; chỉ nhập subset **login + nhân vật** (đã normalize) hoặc để QA tạo mới. Ưu: nhanh tới Gate 2 dev. Nhược: mất ít nhất một phần lịch sử / zen / item khó map.

**Khuyến nghị spike:** bắt đầu **B** để unblock **Gate 2** và song song chứng minh **một luồng A** nhỏ (vd. chỉ account + một character demo) trong `tools/db-migrate/`.

*(Khi nhóm đồng ý → chọn A hoặc B trong checklist Phase 2 và ghi vào wiki/PR fork OpenMU.)*

---

## 4. Công cụ & bước kỹ thuật

Tham chiếu: [`tools/db-migrate/README.md`](../../tools/db-migrate/README.md).

1. Restore **`.bak`** trên MSSQL (hoặc SSMS script `CREATE TABLE` nếu không có instance).  
2. Chạy **`takumi-mssql-inspect`** (read-only) CSV cột MSSQL `dbo` và **`takumi-pg-inspect`** CSV cùng định dạng cho schema Postgres OpenMU **`data`** / **`config`** — đối chiếu với **`EntityDataContextModelSnapshot`** / ETL.  
3. Điền mapping: **[`PHASE2-MAPPING-TEMPLATE.csv`](PHASE2-MAPPING-TEMPLATE.csv)** — **62 proc + 51 bảng** đã có một dòng mỗi object (theo [`TAKUMI-SQL-BACKLOG.md`](TAKUMI-SQL-BACKLOG.md)); chỉnh `openmu_or_plugin` / `parity_status` / `notes` sau khi so CSV inspector. Clone sang Sheet nếu cần thêm cột.  
4. **TODO:** script ETL (dotnet/Npgsql) chỉ đọc MSSQL, ghi **Postgres staging** — không chạy trên prod.  

---

## 5. Việc còn mở Phase 2

- [ ] Spreadsheet: một dòng một **bảng/proc legacy** × cột **OpenMU / plugin / WONTFIX**. **[`PHASE2-MAPPING-TEMPLATE.csv`](PHASE2-MAPPING-TEMPLATE.csv)** đã seed đại diện từ [`TAKUMI-SQL-BACKLOG.md`](TAKUMI-SQL-BACKLOG.md) — bổ sung proc/bảng còn lại sau khi có CSV từ `takumi-mssql-inspect`.  
- [ ] Chạy thử **migration pipeline** và **Gate 2** staging.  

**Liên kết:** [`TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](../TAKUMI-MIGRATION-OPENMU-CHECKLIST.md) Phase 2; [`TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](../TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) §11.
