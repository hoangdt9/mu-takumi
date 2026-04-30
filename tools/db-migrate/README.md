# DB migrate — Takumi (MSSQL) → OpenMU (PostgreSQL)

Quy ước layout **Phase 2** (xem [`docs/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](../../docs/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md)). **ETL Postgres** vẫn TODO; trong repo đã có **công cụ đọc-only** để dump schema MSSQL sau khi restore `MuOnline.bak`.

## Mục tiêu

- Đọc nguồn: ODBC / backup **`MuOnline.bak`** hoặc instance MSSQL dev.
- Xuất đích: **PostgreSQL** mà **`OpenMU`** dùng (connection string của `deploy/all-in-one` hoặc local dev).

## Layout

```
tools/db-migrate/
  README.md              (file này)
  dotnet/
    Takumi.MssqlInspect/ (console: read-only schema → stdout / file)
  schemas/               (optional) export CSV/markdown đã redirect — không chứa password
```

### Spreadsheet mapping (seed)

[`docs/takumi-game-spec/PHASE2-MAPPING-TEMPLATE.csv`](../../docs/takumi-game-spec/PHASE2-MAPPING-TEMPLATE.csv) — cột: `kind,legacy_name,openmu_or_plugin,parity_status,notes`. Copy sang Sheet hoặc mở rộng trong repo.

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

Chiến lược entity / ADR: **[`docs/takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md`](../../docs/takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md)**.

## Quy tắc

- Không commit **connection string có mật khẩu**; dùng biến môi trường hoặc `appsettings.Local.json` (gitignored).
- Đặt **golden sample** nhỏ (vài row anonymized) nếu cần test — không đụng `.bak` đầy đủ trên CI.
- Thư mục `schemas/` nên **gitignore** nếu export chứa default expression nhạy cảm; hoặc chỉ commit bản đã rút gọn.
