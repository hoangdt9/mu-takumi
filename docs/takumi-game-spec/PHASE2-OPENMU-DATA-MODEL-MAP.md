# Phase 2 — SQL Takumi legacy ↔ OpenMU (PostgreSQL / EF)

**Mục đích:** Bổ sung **migration dữ liệu** sau inventory proc/bảng trong [`TAKUMI-SQL-BACKLOG.md`](TAKUMI-SQL-BACKLOG.md). Repo **OpenMU** tham chiếu: ví dụ `OpenMU/` trên máy dev (`src/DataModel`, `src/Persistence/EntityFramework`).

**Chân lý schema đầy đủ:** vẫn là **`MuServer/7.DataBase/MuOnline.bak`** (+ restore MSSQL); `SQLUp.sql` chỉ là **patch custom** đã áp vào một bản MU cụ thể.

## 0 — Nguồn sự thật, ưu tiên merge, và “đa schema” Postgres

### 0.1 Không sót dữ liệu legacy (mặc định dự án Takumi)

- **Nguồn import** cho dữ liệu đã tồn tại trên server cũ là **MSSQL** đã restore (`dbo.*`, proc semantics trong [`TAKUMI-SQL-BACKLOG.md`](TAKUMI-SQL-BACKLOG.md) và bảng đầy đủ trong [`PHASE2-MAPPING-TEMPLATE.csv`](PHASE2-MAPPING-TEMPLATE.csv) phần `LEGACY_*`).
- Mỗi bảng `LEGACY_TABLE` cần một trong các đích: **map sang bảng OpenMU / bảng plugin Postgres / lưu staging audit / ghi quyết định WONTFIX có lý do** — tránh “lỡ” không nhắc tới.
- Kiểm tra sau ETL (khi có script): so **số bảng nguồn**, **số dòng tối thiểu** (hoặc sample hash) theo từng bảng quan trọng; log bảng nguồn **0 row** vs **bỏ qua**.

### 0.2 Trùng tên hoặc trùng “khái niệm” — ưu tiên **giá trị nghiệp vụ từ MSSQL**

- Nếu cùng tên bảng/cột giữa legacy và OpenMU nhưng **kiểu khác** (vd. `int` map ID vs `uuid` OpenMU): **ưu tiên bảo toàn thông tin từ MSSQL** khi transform (ghi vào cột đích tương đương, extension table, hoặc JSON/metadata plugin), không âm thầm lấy default OpenMU làm sự thật.
- **Ngoại lệ an toàn hệ thống:** khóa chính, FK, mật khẩu (hash lại BCrypt), constraint OpenMU — có thể **sinh giá trị mới** (Guid) nhưng phải **giữ bản đồ** sang id legacy để không mất liên kết.
- Bản **fresh OpenMU** (world config) có thể giữ nguyên từ bộ cài; chỉ phần **account/character/inventory/…** mang từ player cần áp quy tắc “MSSQL wins” ở trên.

### 0.3 Postgres OpenMU là **đa schema** (không phải “đa ngôn ngữ” DB)

- OpenMU dùng nhiều **schema** trong **một** database PostgreSQL: `data`, `config`, `friend`, `guild` (xem `SchemaNames.cs`) — đó là **tách vùng dữ liệu**, không có nghĩa DB tự “multi-language” như i18n client.
- Chuỗi hiển thị ngôn ngữ / file client / `Data/*.txt` là lớp **game content**; có thể phải chỉnh khi parity client Takumi, **độc lập** với quyết định Phase 2 nhưng vẫn phải **đồng bộ** khi vào Phase 4.

### 0.4 Sau Phase 2 — các bước C++/OpenMU **có thể bắt chỉnh lại ETL / schema**

- Khi implement **GameServer** (.NET) và so packet với client Takumi, có thể phát hiện thiếu cột, thiếu bảng plugin, hoặc map ID world sai → **quay lại** mapping + ETL/staging là bình thường.
- Coi **PHASE2 mapping + ETL** là phiên bản **lặp** theo gate (Gate 2 → Gate 4), không phải một lần xong vĩnh viễn.

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
3. Điền mapping: **[`PHASE2-MAPPING-TEMPLATE.csv`](PHASE2-MAPPING-TEMPLATE.csv)** — một file **236 dòng** (header + 62 proc + 61 bảng `dbo` + 101 bảng OpenMU EF + 11 heuristic); chỉnh cột sau khi so CSV inspector. Regenerate slice: `--mapping-rows` / `--mapping-openmu-all` trong [`tools/db-migrate/README.md`](../../tools/db-migrate/README.md).  
4. **TODO:** script ETL (dotnet/Npgsql) chỉ đọc MSSQL, ghi **Postgres staging** — không chạy trên prod.  

---

## 5. Việc còn mở Phase 2

- [ ] Spreadsheet: điền **`PHASE2-MAPPING-TEMPLATE.csv`** — file **đầy đủ** gồm `LEGACY_PROC` (62) + `LEGACY_TABLE` (61 `dbo`) + `OPENMU_TABLE` (101) + `HEURISTIC_VERIFY` (11); chỉnh `openmu_or_plugin` / `parity_status` sau diff inspector.  
- [ ] **Kiểm tra không sót nguồn:** mỗi `LEGACY_TABLE` có đích hoặc WONTFIX ghi rõ; sau ETL có smoke so row count/sample (mục **§0** trên file này).
- [ ] Chạy thử **migration pipeline** và **Gate 2** staging.  

**Liên kết:** [`TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](../TAKUMI-MIGRATION-OPENMU-CHECKLIST.md) Phase 2; [`TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](../TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) §11.
