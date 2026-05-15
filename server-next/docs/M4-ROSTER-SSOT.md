# M4 — Roster source of truth (SSOT) — decision record

Last updated: 2026-05-16

## Decision (frozen for current `server-next` iteration)

1. **Runtime SSOT (khuyến nghị):** Postgres **`character_roster`** + **`inventory_slot`** when `TAKUMI_ROSTER_DB_SYNC=1` and `TAKUMI_ROSTER_DB_PRIMARY=1`. Host load DB trước; JSON không bắt buộc.

2. **Dev cache (tuỳ chọn):** **`takumi-roster/<account>.json`** — backfill một lần qua `TAKUMI_MIGRATE_ROSTER_JSON=1`, không phải nguồn bắt buộc lúc chạy production.

3. **Postgres `public.character_roster`:** mirror / primary store:
   - **Upsert** (replace-all rows per account) after successful JSON save (`CharacterRosterMirrorWriter`).
   - **Optional overlay on login** when `TAKUMI_ROSTER_DB_SYNC=1` and `TAKUMI_ROSTER_DB_MERGE_MODE` is not `json` (`CharacterRosterMerge.ApplyDbOverlay`).

4. **Inventory SSOT:** **`inventory_slot`** (12-byte wire). Import bulk từ **`inventory_staging`** (`TAKUMI_IMPORT_INVENTORY_STAGING=1`) — **không** đọc từ roster JSON.

5. **Runtime EF / `takumi_runtime.character`:** may exist for broader product goals; minimal hosts dùng `character_roster` + `character_domain`.

### Vitals overlay (M7)

When `TAKUMI_ROSTER_DB_SYNC=1` and merge mode is not `json`, login merge copies **`current_hp` / `max_hp` / `current_mp` / `max_mp` / `zen`** from `character_roster` into the in-memory roster (same pass as map/xy). JSON file remains written first on save; Postgres upsert follows the JSON snapshot. **First join** with unset vitals (`max_hp == 0`): hosts seed from the **`F3 03`** wire they just sent (`JoinMapVitalsSeed`) so disconnect / periodic flush persist non-zero stats.

## Optional: Postgres-first load (`TAKUMI_ROSTER_DB_PRIMARY=1`)

Requires **`TAKUMI_ROSTER_DB_SYNC=1`**. On login, hosts load **`character_roster`** first; if the account has rows, JSON is not read and DB overlay is skipped. If DB is empty, behavior falls back to JSON + optional overlay (`TAKUMI_ROSTER_DB_MERGE_MODE`).

On save, Postgres upsert still runs. JSON export is skipped unless **`TAKUMI_ROSTER_JSON_EXPORT=1`** (useful as a local cache for QA).

Implementation: **`CharacterRosterBootstrap`**, **`CharacterRosterHostLoad`** (game TCP).

## Postgres-only path (minimal hosts, 2026-05-16)

When **`TAKUMI_ROSTER_DB_PRIMARY=1`** + **`TAKUMI_CHARACTER_DOMAIN_SYNC=1`**:

1. Load order: `character_roster` → `character_domain` (if roster empty) → JSON fallback.
2. Save: upsert `character_roster` → async mirror `character_domain` (`CharacterDomainMirrorWriter`).
3. Optional staging: **`TAKUMI_IMPORT_CHARACTER_STAGING=1`** fills both tables from `character_staging` at startup.

EF **`takumi_runtime.character`** (full `Takumi.Server.Host`) remains a separate product path.

## Related

- `docs/M4-WORLD-POSITION-CHECKLIST.md` — engineering checklist.
- `docs/M6-GAME-TCP-CHECKLIST.md` — split TCP minimal-login (same roster files + mirror).
