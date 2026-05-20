# M4 — Roster source of truth (SSOT) — decision record

Last updated: 2026-05-18

## Decision (frozen for current `server-next` iteration)

### Runtime default (Docker / `env.defaults`)

With **`TAKUMI_ROSTER_DB_SYNC=1`** + **`TAKUMI_ROSTER_DB_PRIMARY=1`** (compose default):

1. **Load:** `character_roster` → `character_domain` (if empty) → JSON fallback.
2. **Save:** upsert `character_roster` (+ mirror `character_domain` when `TAKUMI_CHARACTER_DOMAIN_SYNC=1`).
3. **JSON:** skipped on save unless **`TAKUMI_ROSTER_JSON_EXPORT=1`**.
4. **Boot log:** `[roster-ssot] mode=postgres-primary …` once per process (`CharacterRosterSsoT`).

### Legacy / dev without DB primary

1. **Authoritative roster for accounts we fully control:** **`takumi-roster/<account>.json`** on disk (or `TAKUMI_ROSTER_DIR`). The hosts read/write this file on login, character ops, walk/move, and disconnect flush.

2. **Postgres `public.character_roster`:** a **mirror** of the JSON snapshot:
   - **Upsert** (replace-all rows per account) after successful JSON save (`CharacterRosterMirrorWriter`).
   - **Optional overlay on login** when `TAKUMI_ROSTER_DB_SYNC=1` and `TAKUMI_ROSTER_DB_MERGE_MODE` is not `json` (`CharacterRosterMerge.ApplyDbOverlay`).

3. **Runtime EF / `takumi_runtime.character`:** may exist for broader product goals (see `IMPLEMENTATION-CHECKLIST.md` §Done); it is **not** the roster SSOT for minimal login hosts until an explicit migration project wires **one** write path.

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

- `character/M4-WORLD-POSITION-CHECKLIST.md` — engineering checklist.
- `character/M6-GAME-TCP-CHECKLIST.md` — split TCP minimal-login (same roster files + mirror).
