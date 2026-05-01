# DB migrate — Takumi (MSSQL) → OpenMU (PostgreSQL)

Quy ước layout **Phase 2** (xem [`docs/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](../../docs/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md)). **ETL Postgres** vẫn TODO; trong repo có **hai công cụ đọc-only** (MSSQL legacy + Postgres OpenMU) cùng định dạng CSV để so cột.

## Mục tiêu

- Đọc nguồn: ODBC / backup **`MuOnline.bak`** hoặc instance MSSQL dev.
- Xuất đích: **PostgreSQL** mà **`OpenMU`** dùng (connection string của `deploy/all-in-one` hoặc local dev).

## Layout

```
tools/db-migrate/
  README.md                    (file này)
  dotnet/
    Takumi.DbTools.slnx        (MssqlInspect + PgInspect)
    Takumi.MssqlInspect/       takumi-mssql-inspect
    Takumi.PgInspect/          takumi-pg-inspect (OpenMU / Postgres)
  schemas/                     (optional) export CSV redirect — không chứa password
```

### Spreadsheet mapping (seed)

[`docs/takumi-game-spec/PHASE2-MAPPING-TEMPLATE.csv`](../../docs/takumi-game-spec/PHASE2-MAPPING-TEMPLATE.csv) — cột: `kind,legacy_name,openmu_or_plugin,parity_status,notes`. Copy sang Sheet hoặc mở rộng trong repo; **đối chiếu đầy đủ** 62 `WZ_*` + 51 bảng heuristic trong [`TAKUMI-SQL-BACKLOG.md`](../../docs/takumi-game-spec/TAKUMI-SQL-BACKLOG.md) và bổ sung dòng khi rà `.bak`/inspector.

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

# Một bảng: CSV hoặc markdown
dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- --table Character
dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- --table Character --markdown

# Schema khác dbo
dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect -- --schema dbo --tables
```

Hoặc `cd tools/db-migrate/dotnet/Takumi.MssqlInspect` rồi `dotnet run -- --help` (không cần `--project`).

Release binary: `dotnet publish -c Release` → chạy `takumi-mssql-inspect` trong thư mục publish.

Build cả hai tool: `dotnet build tools/db-migrate/dotnet/Takumi.DbTools.slnx`

## `takumi-pg-inspect` (read-only, Postgres)

**.NET 10**, `Npgsql`. Cùng header CSV với MSSQL: `table_schema,table_name,column_name,data_type,max_len,nullable,default` — tiện diff với legacy (tên bảng/schema khác nhau vẫn so được thủ công hoặc paste Sheet).

**Connection:** `TAKUMI_PG_CONNECTION` hoặc `--connection` (chuỗi Npgsql, ví dụ `Host=127.0.0.1;Port=5439;Database=openmu;Username=postgres;Password=***`).

OpenMU thường dùng schema EF như **`AccountData`**, **`Configuration`** — chỉ định **`--schema AccountData`** (mặc định `public`).

```bash
dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --help

TAKUMI_PG_CONNECTION="Host=127.0.0.1;Port=5439;Database=openmu;Username=postgres;Password=***" \
  dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --schema AccountData \
  > tools/db-migrate/schemas/openmu-accountdata-columns.csv

dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --schema AccountData --tables
dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --schema AccountData --table Character
```

Chiến lược entity / ADR: **[`docs/takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md`](../../docs/takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md)**.

## Quy tắc

- Không commit **connection string có mật khẩu**; dùng biến môi trường hoặc `appsettings.Local.json` (gitignored).
- Đặt **golden sample** nhỏ (vài row anonymized) nếu cần test — không đụng `.bak` đầy đủ trên CI.
- Thư mục `schemas/` nên **gitignore** nếu export chứa default expression nhạy cảm; hoặc chỉ commit bản đã rút gọn.
