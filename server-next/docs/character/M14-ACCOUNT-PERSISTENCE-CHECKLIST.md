# M14 — Account persistence (Postgres)

**Mục tiêu:** Login (`F1 01`) và đăng ký in-game (`C1 D3 05`) đọc/ghi bảng runtime `public.account`, tương đương tinh thần **`MEMB_INFO`** (legacy JoinServer) và **`Account`** (OpenMU — BCrypt).

**Phạm vi minimal hosts:** `LegacyLoginHostRunner`, `GamePortMinimalSession` (`Takumi.Server.GameHost`).

---

## Legacy / OpenMU mapping

| Legacy (`MEMB_INFO`) | OpenMU | `server-next` (`public.account`) |
|----------------------|--------|----------------------------------|
| `memb___id` | `LoginName` | `account_login` (PK, lower-case) |
| `memb__pwd` (plaintext) | `PasswordHash` (BCrypt) | `password_hash` |
| `sno__numb` (7-digit) | — | `security_code` |
| `tel__numb` | — | `phone` |
| `bloc_code` | lock flags | `bloc_code` (reserved, default 0) |

In-game register wire: `CB_DangKyInGame` → `C1 D3 05` (59 bytes) — account 11, password 21, security 8, phone 14.

---

## SQL & env

- [x] **`sql/init/010_account.sql`** — bảng `public.account` + index `phone`.
- [x] **`sql/init/000_legacy_schema.sql`** — `CREATE SCHEMA takumi_legacy` (ETL mirror).
- [x] Áp dụng trên volume cũ: `./scripts/db/apply-sql.sh 'postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime'` (chạy toàn bộ `sql/init/*.sql` theo thứ tự tên).
- [x] **`TAKUMI_ACCOUNT_DB=1`** trong `env.defaults` (cần connection string Postgres giống roster).
- [x] **`TAKUMI_LEGACY_PROMOTE=1`** + **`TAKUMI_DROP_EF_RUNTIME_SCHEMA=1`** — startup promote `takumi_legacy` / `takumi_staging` → `public.*`, migrate rồi **DROP SCHEMA** `takumi_runtime` (EF duplicate; **không** phải tên database `takumi_runtime`).
- [x] One-shot: `./scripts/db/promote-legacy-schema.sh` (hoặc `TAKUMI_LEGACY_PROMOTE_ONLY=1` + GameHost).
- [x] **`TakumiPostgresMirror.InitAccountDbIfEnabled()`** — tạo `PostgresAccountRepository`.
- [x] Seed dev: `TAKUMI_ACCOUNTS` (`test:test`, `admin:admin`) upsert vào DB lúc startup nếu chưa có (`AccountCredentialGate.SeedEnvAccountsAsync`).

**DBeaver:** chỉ dùng **`public.account`** cho login/register M14. Không dùng `takumi_runtime.account` (schema EF cũ — bị drop sau promote).

---

## Code

- [x] **`PostgresAccountRepository`** — `TryCreate`, `VerifyPassword`, `Exists`, `TryResetPassword` (security + phone).
- [x] **`AccountPasswordHasher`** — BCrypt work factor 11.
- [x] **`AccountCredentialGate`** — login: DB trước, fallback env dict; register: INSERT + cập nhật dict in-memory (cùng session).
- [x] **`GameInGameRegistration`** — `IsValidRequest`; register path async qua gate.
- [x] **`GamePortMinimalSession`** — `F1 01` + `D3 05` dùng gate.
- [x] **`LegacyLoginHostRunner`** — `F1 01` dùng gate.
- [x] **`LegacySchemaPromoter`** — promote staging + drop EF schema `takumi_runtime`.

---

**QA APK (sau):** [`../../docs/QA-MILESTONE.md`](../../docs/QA-MILESTONE.md) — đăng ký/login thiết bị.

---

## Open / follow-up

- [ ] Wire client **reset password** (`TypeSend` / JoinServer `0x0B`) → `TryResetPasswordAsync`.
- [x] Import `MEMB_INFO` từ `takumi_legacy` staging → `account` (startup `LegacySchemaPromoter` + ETL `staging-login-path`).
- [ ] `bloc_code` / ban / lock parity JoinServer.
- [ ] Full **`Takumi.Server.Host`** EF `Account` entity — dùng chung bảng hoặc migrate schema.
- [ ] Integration test Postgres khi `TEST_PG_CONNECTION_STRING` set (`AccountPostgresTests`).

---

## Related docs

- **`milestones/IMPLEMENTATION-CHECKLIST.md`** — mục M14 + Data & Migration.
- **`protocol/LOGIN-WIRE-FORMAT.md`** — `F1 01` layout.
- Legacy: `Source/3.JoinServer/JoinServer/JoinServerProtocol.cpp` (`GJRegistroMainRecv`, `INSERT MEMB_INFO`).
