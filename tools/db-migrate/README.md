# DB migrate — Takumi (MSSQL) → OpenMU (PostgreSQL)

Placeholder cho **Phase 2** của [`docs/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](../../docs/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md). Chưa implement script trong repo Takumi — chỉ quy ước layout.

## Mục tiêu

- Đọc nguồn: ODBC / backup **`MuOnline.bak`** hoặc instance MSSQL dev.
- Xuất đích: **PostgreSQL** mà **`OpenMU`** dùng (connection string của `deploy/all-in-one` hoặc local dev).

## Đề xuất layout (tùy team)

```
tools/db-migrate/
  README.md           (file này)
  dotnet/             (optional) console read MSSQL → write Npgsql)
  schemas/            (optional) DDL diff exports, không chứa password
```

## Quy tắc

- Không commit **connection string có mật khẩu**; dùng biến môi trường hoặc `appsettings.Local.json` (gitignored).
- Đặt **golden sample** nhỏ (vài row anonymized) nếu cần test — không đụng `.bak` đầy đủ trên CI.

Đọc chiến lược và mapping entity: **[`docs/takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md`](../../docs/takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md)**.
