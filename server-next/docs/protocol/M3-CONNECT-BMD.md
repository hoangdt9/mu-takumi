# M3 — Connect server (`1.ConnectServer`) parity

**Scope:** Standalone TCP connect port (default **44605**) handled by **`Takumi.Server.Connect`** (`ConnectMiniServer`, `ConnectServerPacketClassifier`). **`LegacyLoginHost`** builds the **`C2 F4 06`** list bytes via **`ConnectServerList602`** and passes them into the mini server.

## Wire requests handled

| Client request | Response |
|----------------|----------|
| `C1` patch probe (head `0x02` at `[2]`, length ≥ 6) | `ConnectPatchWire602.BuildPatchVersionOkay()` → `C1 04 02 00` |
| `C1` main code `0x05` (Takumi / OpenMU patch path) | Same patch OK |
| `C1 F4 02` or `C1 F4 06` | `ServerList602` bytes (`C2 F4 06` list), or **`C1 F4 05` busy** if QA flag set |
| `C1 F4 03` | `ConnectServerInfo602.Build(publicHost, loginPort)` |

**Coalesced reads:** `TryFindFirstRequestOfKind` finds the first matching frame of a given kind so a buffer that leads with `F4 03` before `F4 06` still answers list when scanning for **ServerList** (legacy behavior checked `F4 06` before `F4 03`).

**Priority per read:** patch → list → info (patch answered first if multiple kinds appear in one buffer).

## `ServerList.bmd` and connect indices

Takumi maps each connect **wire id** to a **BMD group** via **`connectIndex / 20`** (`ServerListManager.cpp`). The client keeps **at most 15** sub-server slots per group (`SLM_MAX_SERVER_COUNT`) but indexes non-PVP data with **`(connectIndex % 20) + 1`**. Sending more than **15** distinct indices in the same group where **`connectIndex % 20`** covers values **≥ 15** can crash native clients (out-of-bounds read).

**Safe rule:** per group `g`, use wire ids **`g*20` … `g*20+14`** only (15 slots). The default preset in **`LegacyLoginHost`** uses **`0..14`**, **`20..34`**, plus **`40`**, **`41`** (two extras in group 2) — still ≤15 per group.

**Overrides:**

- **`TAKUMI_CS_CONNECT_IDS`** — CSV of wire ids (validated by **`ConnectServerList602`**).
- **`TAKUMI_CS_CONNECT_BASE`** + **`TAKUMI_CS_CONNECT_COUNT`** — sequential ids from base (avoid base **0** for typical BMDs with no group 0).

## QA env flags

- **`TAKUMI_CONNECT_RETURN_BUSY=1`** (or `true` / `yes`) — list requests get **`ConnectServerBusy602`** (`C1 05 F4 05` + index) instead of the list.
- **`TAKUMI_CONNECT_BUSY_INDEX`** — byte appended after `F4 05` (default **0**).

## Projects

- **`src/Takumi.Server.Connect/`** — classifier, TCP mini server, keepalive helper.
- **`src/Takumi.Server.Protocol/`** — `ConnectServerList602`, `ConnectServerInfo602`, `ConnectPatchWire602`, `ConnectServerBusy602`.
