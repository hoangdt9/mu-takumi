# M4 — Roster source of truth (SSOT) — decision record

Last updated: 2026-05-16

## Decision (frozen for current `server-next` iteration)

1. **Authoritative roster for accounts we fully control:** **`takumi-roster/<account>.json`** on disk (or `TAKUMI_ROSTER_DIR`). The hosts read/write this file on login, character ops, walk/move, and disconnect flush.

2. **Postgres `public.character_roster`:** a **mirror** of the JSON snapshot:
   - **Upsert** (replace-all rows per account) after successful JSON save (`CharacterRosterMirrorWriter`).
   - **Optional overlay on login** when `TAKUMI_ROSTER_DB_SYNC=1` and `TAKUMI_ROSTER_DB_MERGE_MODE` is not `json` (`CharacterRosterMerge.ApplyDbOverlay`).

3. **Runtime EF / `takumi_runtime.character`:** may exist for broader product goals (see `IMPLEMENTATION-CHECKLIST.md` §Done); it is **not** the roster SSOT for minimal login hosts until an explicit migration project wires **one** write path.

## What “Postgres-only SSOT” would require later

- Single writer API used by **both** `LegacyLoginHost` / `GamePortMinimalSession` and any importer.
- Conflict rules between **JSON**, **`character_roster`**, and **`character`** domain rows.
- Backfill / cutover plan for existing LAN QA machines (volumes with JSON only).

Track implementation under **`docs/M4-WORLD-POSITION-CHECKLIST.md`** §Importer / **`IMPLEMENTATION-CHECKLIST.md`** §Next High — **not** a prerequisite for **M5** join/ticket work (`docs/M5-JOIN-HANDOFF-CHECKLIST.md`).

## Related

- `docs/M4-WORLD-POSITION-CHECKLIST.md` — engineering checklist.
- `docs/M6-GAME-TCP-CHECKLIST.md` — split TCP minimal-login (same roster files + mirror).
