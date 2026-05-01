# DB migrate — Takumi (MSSQL) → OpenMU (PostgreSQL)

Quy ước layout **Phase 2** (xem [`docs/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](../../docs/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md)). Job **copy sang Postgres/OpenMU** (ETL đầy đủ) **TODO**. Repo có **hai inspector read-only** (MSSQL + Postgres) cùng CSV cột, **`takumi-etl`** (smoke-test DB + holder cho loader sau), và script **`regen-mapping-slices.sh`**.

## Mục tiêu

- Đọc nguồn: ODBC / backup **`MuOnline.bak`** hoặc instance MSSQL dev.
- Xuất đích: **PostgreSQL** mà **`OpenMU`** dùng (connection string của `deploy/all-in-one` hoặc local dev).

## Layout

```
tools/db-migrate/
  README.md                    (file này)
  regen-mapping-slices.sh      một lần chạy: slice mapping + row-count (cần env)
  dotnet/
    Takumi.DbTools.slnx        (Etl + MssqlInspect + PgInspect)
    Takumi.Etl/                takumi-etl (`check-sources` — loads TODO)
    Takumi.MssqlInspect/       takumi-mssql-inspect
    Takumi.PgInspect/          takumi-pg-inspect (OpenMU / Postgres)
  schemas/                     export CSV redirect — không chứa password
```

**Nguyên tắc dữ liệu:** khi ETL/trùng tên, **ưu tiên bảo toàn nội dung từ MSSQL** (legacy là sự thật cho account/char/item); Postgres OpenMU là **đa schema** (`data`, `config`, …) trong một DB — không nhầm với “multi-language”. Chi tiết: [`PHASE2-OPENMU-DATA-MODEL-MAP.md`](../../docs/takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md) **§0**.

### Spreadsheet mapping — **đủ tầng** (proc + dbo + OpenMU EF + heuristic)

[`docs/takumi-game-spec/PHASE2-MAPPING-TEMPLATE.csv`](../../docs/takumi-game-spec/PHASE2-MAPPING-TEMPLATE.csv) — **236 dòng** (bao gồm header): `LEGACY_PROC` (62), `LEGACY_TABLE` (61 `dbo` từ `.bak`), `OPENMU_TABLE` (101 bảng `data`/`config`/`friend`/`guild`), `HEURISTIC_VERIFY` (11 tên chỉ từ grep C++). Clone sang Sheet để điền `openmu_or_plugin` / `parity_status`.

**Regenerate slice từ DB (sau khi restore / OpenMU migrate):**

```bash
export TAKUMI_MSSQL_CONNECTION="..."
export TAKUMI_PG_CONNECTION="..."
bash tools/db-migrate/regen-mapping-slices.sh
```

Tương đương từng lệnh:

```bash
# MSSQL: mọi bảng dbo → CSV (chỉ phần LEGACY_TABLE + header)
dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- --mapping-rows \
  > docs/takumi-game-spec/PHASE2-MAPPING-MSSQL-DBO-AUTO.csv

# Postgres OpenMU: đủ 4 schema EF
dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --mapping-openmu-all \
  > docs/takumi-game-spec/PHASE2-MAPPING-OPENMU-EF-TABLES-FULL.csv
```

Sau đó cập nhật `PHASE2-MAPPING-TEMPLATE.csv` (merge với `LEGACY_PROC` + `HEURISTIC_VERIFY` từ backlog — xem [`TAKUMI-SQL-BACKLOG.md`](../../docs/takumi-game-spec/TAKUMI-SQL-BACKLOG.md)).

**Row counts (đối chiếu trước/sau ETL, không cần đếm tay):** cả hai tool xuất CSV cùng cột `table_schema,table_name,row_count`.

- MSSQL: `--row-counts` — số dòng từ `sys.partitions` (mỗi bảng base một dòng).
- Postgres: `--row-counts` / `--row-counts-openmu-all` — `n_live_tup` từ `pg_stat_all_tables` (ước lượng thống kê; chạy `ANALYZE` trên DB nếu cần số “mới” hơn).

```bash
TAKUMI_MSSQL_CONNECTION="..." dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- --row-counts \
  > tools/db-migrate/schemas/mssql-dbo-row-counts.csv
TAKUMI_PG_CONNECTION="..." dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --row-counts-openmu-all \
  > tools/db-migrate/schemas/openmu-all-row-counts.csv
```

## `takumi-mssql-inspect` (read-only)

**.NET 10**, assembly `Microsoft.Data.SqlClient`. **Không** ghi DB; chỉ đọc `INFORMATION_SCHEMA`.

**Connection** (ưu tiên đầu tiên khớp):

1. Biến môi trường **`TAKUMI_MSSQL_CONNECTION`**
2. Tham số **`--connection "Server=...;Database=MuOnline;..."`**

Chạy từ **root repo Takumi** (đường dẫn `--project` tính từ đó):

```bash
dotnet restore tools/db-migrate/dotnet/Takumi.MssqlInspect/Takumi.MssqlInspect.csproj
dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- --help

# Dump toàn bộ cột schema dbo → CSV (một dòng header)
mkdir -p tools/db-migrate/schemas
TAKUMI_MSSQL_CONNECTION="Server=127.0.0.1,1433;Database=MuOnline;User Id=sa;Password=***;TrustServerCertificate=True" \
  dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- > tools/db-migrate/schemas/muonline-dbo-columns.csv

# Chỉ danh sách bảng
dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- --tables

# Số dòng mỗi bảng dbo → CSV (đối chiếu migration)
dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- --row-counts

# Một bảng: CSV hoặc markdown
dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- --table Character
dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- --table Character --markdown

# Schema khác dbo
dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- --schema dbo --tables
```

Hoặc `cd tools/db-migrate/dotnet/Takumi.MssqlInspect` rồi `dotnet run -- --help` (không cần `--project`).

Release binary: `dotnet publish -c Release` → chạy `takumi-mssql-inspect` trong thư mục publish.

Build solution: `dotnet build tools/db-migrate/dotnet/Takumi.DbTools.slnx`

## `takumi-etl` (Phase 2 scaffold)

Chưa ghi Postgres. Lệnh **`check-sources`** mở kết nối MSSQL/OpenMU có trong env (giống inspector) và in `OK`, `FAIL`, hoặc `SKIP`.

**`preview-login-path`** (read-only MSSQL): tìm `MEMB_INFO` + `Character` theo `--schema dbo` (tên bảng match không phân biệt hoa thường), in **số dòng**, **danh sách cột** và một khối gợi ý map sang `data.Account` / `data.Character` (theo PHASE2 §2 — không INSERT).

```bash
TAKUMI_MSSQL_CONNECTION="..." dotnet run --project tools/db-migrate/dotnet/Takumi.Etl -- preview-login-path
dotnet run --project tools/db-migrate/dotnet/Takumi.Etl -- check-sources
```

## `takumi-pg-inspect` (read-only, Postgres)

**.NET 10**, `Npgsql`. Cùng header CSV với MSSQL: `table_schema,table_name,column_name,data_type,max_len,nullable,default` — tiện diff với legacy (tên bảng/schema khác nhau vẫn so được thủ công hoặc paste Sheet).

**Connection:** `TAKUMI_PG_CONNECTION` hoặc `--connection` (chuỗi Npgsql, ví dụ `Host=127.0.0.1;Port=5439;Database=openmu;Username=postgres;Password=***`).

OpenMU: trong Postgres tên schema là **`data`**, **`config`**, **`friend`**, **`guild`** (xem `OpenMU/.../SchemaNames.cs`: `AccountData` → `"data"`, `Configuration` → `"config"`). Với inspector dùng **`--schema data`** (không phải chữ `AccountData`).

```bash
dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --help

TAKUMI_PG_CONNECTION="Host=127.0.0.1;Port=5433;Database=openmu;Username=postgres;Password=***" \
  dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --schema data \
  > tools/db-migrate/schemas/openmu-data-columns.csv

dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --schema data --tables
dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --schema data --table Character
dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --schema config --tables

# Số dòng (pg_stat) — một schema hoặc cả OpenMU
dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --schema data --row-counts
dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --row-counts-openmu-all
```

*(Port `5433` nếu dùng `deploy/all-in-one/docker-compose.override.yml`; mặc định compose có thể là `5432`.)*

Chiến lược entity / ADR: **[`docs/takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md`](../../docs/takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md)**.

## Quy tắc

- Không commit **connection string có mật khẩu**; dùng biến môi trường hoặc `appsettings.Local.json` (gitignored).
- Đặt **golden sample** nhỏ (vài row anonymized) nếu cần test — không đụng `.bak` đầy đủ trên CI.
- Thư mục `schemas/` nên **gitignore** nếu export chứa default expression nhạy cảm; hoặc chỉ commit bản đã rút gọn.
