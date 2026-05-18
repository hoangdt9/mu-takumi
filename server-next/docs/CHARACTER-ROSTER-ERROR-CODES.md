# Character roster error codes (F3 01 / F3 02)

Last updated: 2026-05-18

Wire builders: **`CharacterCreateWire602`**, constants **`CharacterRosterErrorCodes`**, validation **`CharacterRosterOps`** (both `GamePortMinimalSession` and `LegacyLoginHostRunner`).

## Create (`C1 F3 01`)

| Result byte | Client UI | Server when |
|-------------|-----------|-------------|
| `1` | Success | Character created, roster saved |
| `0` | `RECEIVE_CREATE_CHARACTER_FAIL` | Roster full (5), invalid class |
| `2` | `RECEIVE_CREATE_CHARACTER_FAIL2` | Duplicate name, invalid name (length/charset) |

## Delete (`C1 F3 02`)

| Result byte | Client UI | Server when |
|-------------|-----------|-------------|
| `1` | `MESSAGE_DELETE_CHARACTER_SUCCESS` | Removed from roster + DB row deleted |
| `2` | `MESSAGE_STORAGE_RESIDENTWRONG` | Name not found, or resident/captcha bytes non-zero |
| `0` | Guild warning | Not used yet (guild domain OPEN) |
| `3` | Item block | Not used yet (item lock OPEN) |

**Resident field:** client sends 20 bytes after name. Dev builds often send all zeros (no captcha UI) — accepted. Non-zero resident is rejected until captcha hash parity is implemented.

## Limits

- Max **5** characters per account (`CharacterRosterErrorCodes.MaxCharactersPerAccount`).
- Name: ASCII 4–10 chars, printable.
