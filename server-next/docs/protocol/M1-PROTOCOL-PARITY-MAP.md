# M1 — Protocol parity map (Takumi client ↔ `server-next`)

**Purpose:** satisfy checklist **M1** — inventory of wire opcodes the **Takumi** client (`Source/5.Main`) understands after login, plus **legacy vs `server-next`** coverage for the MVP host.

**Sources of truth (update this doc when these change):**

| Area | Path |
|------|------|
| Client RX dispatch (main game stream) | `Source/5.Main/source/WSclient.cpp` — `TranslateProtocol` (`switch (HeadCode)`, ~L13611–L15075) |
| Client RX hook (runs **before** `TranslateProtocol`; `return 1` = consumed) | `Source/5.Main/source/Protocol.cpp` — `ProtocolCoreEx` (~L70+) |
| Client structs / list–join layouts | `Source/5.Main/source/WSclient.h` |
| Client TX macros (plain `C1` head + sub) | `Source/5.Main/source/wsclientinline.h` |
| Android character-select TX | `Source/5.Main/source/android/AndroidNetwork.cpp` — `SendCharacterSelectionPacket` (`0x15` focus, `0x03` enter) |
| Legacy GS reference (not in `server-next`) | `Source/4.GameServer/...` (e.g. `DSProtocol.cpp` uses `0xF3, 0x15` in places) |
| MVP .NET host | `src/Takumi.Server.LegacyLoginHost/Program.cs` |

**Wire envelope (Season 6 style):**

- **`C1`**: single-byte length `PBMSG` — `Code, Size, HeadCode[, …]`.
- **`C2` / `C4`**: `Code, SizeH, SizeL, HeadCode[, …]` (wide messages).
- **`C3` / `C4` (encrypted client→server):** SimpleModulus + MU framing; login uses **`spe.Send(TRUE)`** in `SendRequestLogIn` — see `LOGIN-WIRE-FORMAT.md`.
- **`HeadCode`** in tables below is the **MU head** byte (e.g. `0xF3`), **not** the framing `Code` (`C1`/`C3`).

---

## 1. Login / connect phase — legacy vs `server-next`

| Flow | Client TX (summary) | Client RX (summary) | Legacy stack | `Takumi.Server.LegacyLoginHost` |
|------|---------------------|---------------------|--------------|----------------------------------|
| Patch / version | (connect URL / bootstrap) | `C1` patch / version blobs | Connect | Partial / env-driven |
| Server list | `F4` + sub (via connect or login socket) | `C1 F4 06` list, `F4 03` info, `F4 05` busy | `1.ConnectServer` | **Yes** — `F4 02/06`, `F4 03`, `C1 05` PatchInfo on connect **and** login port |
| Login | `C3` encrypted `F1 01` + creds | `C1 F1 01` result (`Value` = status) | Join/login | **Yes** — decrypt session, `F1 01` |
| Character list | `C1 F3 00` + lang (XOR on Android tail) | `C1 F3 00` slots (`PRECEIVE_CHARACTER_LIST`) | Join/login | **Yes** — DB-backed list |
| Create / delete | `C1 F3 01` / `F3 02` | `C1 F3 01` / `F3 02` ack | Join/login | **Yes** |
| Focus / select / join | Desktop: `F3 03` join; Android: **`F3 15`** then **`F3 03`** | `C1 F1 00` join-server ack; `C1 F3 03` **131-byte** `PRECEIVE_JOIN_MAP_SERVER`; then **`F3 10`** inventory | Join + GS | **Yes** — join + `F3 10` on **same TCP** today |
| Post-join game | `F3 12` finish load, movement `0xD4`, chat `0x00`, … | Full `TranslateProtocol` set (§2) | `4.GameServer` | **Not in host** — needs **`Takumi.Server.Game`** / M6+ |

**Gaps called out by checklist (not M1 scope to implement, only to track):**

- **Game-only TCP** after ticket / second port — client may still work single-socket until M5–M6 land; parity doc must stay aligned with `IMPLEMENTATION-CHECKLIST.md` **M4–M6**.
- **`ProtocolCoreEx`** can **shadow** standard handlers for the same `HeadCode` (custom / season patches). Golden pcaps must note **#ifdef** build flags.

---

## 2. Client RX — `HeadCode` dispatch (`TranslateProtocol`)

Primary `switch (HeadCode)` in `WSclient.cpp`. Values are **game-server → client** heads inside decrypted payload.

### 2.1 Sub-head families (`F1`, `F3`, `F4`, …)

| Head | Subcodes handled (hex) | Representative handlers |
|------|-------------------------|-------------------------|
| **0xF1** | `00` join server, `01` login, `02` logout, `12` create account, `03`–`05` password flows | `ReceiveJoinServer`, login state machine, `ReceiveLogOut`, … |
| **0xF3** | `00` list, `01` create, `02` delete, `03` join map, `04` revival, `05` level up, `06` add point, `07` damage, `08` PK, `10` inventory, `11` magic list, `13` equipment, `14` modify item, `20` summon life, `22` WT time, `23` soccer score, `24` WT match, `25` soccer goal, `30` option, `40` server command, `50`–`53` master skill block | See `case 0xF3` in `WSclient.cpp` |
| **0xF4** | `06` server list, `03` server connect, `05` busy | `ReceiveServerList`, `ReceiveServerConnect`, … |

**Note:** Android sends **`F3 15`** as a **client TX** “focus” step; that subcode is **not** in the `TranslateProtocol` **`F3` RX** `switch` — server answers with **`F3 03`** join payload. Legacy GS code may emit **`F3 15`** in some paths (`DSProtocol.cpp`).

### 2.2 Single-byte heads (world / combat / economy)

Macro aliases from `wsclientinline.h`: **`PACKET_MOVE = 0xD4`**, **`PACKET_POSITION = 0x15`**, **`PACKET_ATTACK = 0x11`**, **`PACKET_MAGIC_ATTACK = 0xDB`**.

| Head | Handler / meaning |
|------|-------------------|
| `0x00` | `ReceiveChat` |
| `0x01` | `ReceiveChatKey` |
| `0x02` | `ReceiveChatWhisper` |
| `0x03` | `ReceiveCheckSumRequest` |
| `0x0B` | `ReceiveEvent` |
| `0x0C` | `ReceiveChatWhisperResult` |
| `0x0D` | `ReceiveNotice` |
| `0x0F` | `ReceiveWeather` |
| **`0xD4`** | `ReceiveMoveCharacter` (**`PACKET_MOVE`**) |
| **`0x15`** | `ReceiveMovePosition` (**`PACKET_POSITION`**) |
| `0x12` | `ReceiveCreatePlayerViewport` |
| `0x13` | `ReceiveCreateMonsterViewport` |
| `0x1F` | `ReceiveCreateSummonViewport` |
| `0x45` | `ReceiveCreateTransformViewport` |
| `0x14` | `ReceiveDeleteCharacterViewport` |
| `0x20` | `ReceiveCreateItemViewport` |
| `0x21` | `ReceiveDeleteItemViewport` |
| `0x22` | `ReceiveGetItem` |
| `0x23` | `ReceiveDropItem` |
| `0x24` | `ReceiveEquipmentItem` |
| `0x25` | `ReceiveChangePlayer` |
| **`0x11`** | `ReceiveAttackDamage` (**`PACKET_ATTACK`**) |
| `0x18` | `ReceiveAction` |
| `0x19` | `ReceiveMagic` |
| `0x69` | `ReceiveMonsterSkill` |
| `0x1A` | `ReceiveMagicPosition` |
| `0x1E` | `ReceiveMagicContinue` |
| `0x1B` | `ReceiveMagicFinish` |
| `0x07` | `ReceiveSkillStatus` |
| `0x16` / `0x9C` | `ReceiveDieExp` / large |
| `0x17` | `ReceiveDie` |
| `0x2A` | `ReceiveDurability` |
| `0x26` | `ReceiveLife` |
| `0x27` | `ReceiveMana` |
| `0x28` | `ReceiveDeleteInventory` |
| `0x29` | `ReceiveHelperItem` |
| `0x2C` | `ReceiveUseStateItem` |
| `0x30` | `ReceiveTalk` |
| `0x31`–`0x3D` | Trade sequence (`ReceiveTradeInventory` … `ReceiveTradeExit`) |
| `0x32`–`0x34` | `ReceiveBuy`, `ReceiveSell`, `ReceiveRepair` |
| `0x36`–`0x3D` | Trade / gold / result / exit |
| `0x1C` | `ReceiveTeleport` |
| `0x40`–`0x44`, `0x46`, `0x47`, `0x48` | Party + effects |
| `0x50`–`0x56`, `0x5D`, `0x60`–`0x64`, `0x65`–`0x66`, `0xE1`, `0xE5`–`0xE6`, `0xEB`, `0x67`, `0xE9`, `0xBC` | Guild / union / gem mix |
| `0x68` | `ReceivePreviewPort` |
| `0x71` | `ReceivePing` |
| `0x81`–`0x83`, `0x86`–`0x87` | Storage / mix |
| `0x8E` | Sub `0x01` checksum, `0x03` move map |
| `0x90`–`0x9F`, `0x9A`–`0x9B`, `0x94`–`0x96`, `0x99`, `0x9D`–`0x9E` | Events / Devil Square / scratch / sound |
| `0xA0`–`0xA4` | Quest |
| `0xF6` | Quest / progress subs (`0x03`, `0x0A`–`0x1B`, …) |
| `0xF8` | Gens (`#ifdef ASG_ADD_GENS_SYSTEM`) |
| `0xF9` | NPC dialog UI (`0x01`) |
| `0xA7`–`0xA9` | Pet |
| `0xAA` | Duel (many subs) |
| `0xF7` | Empire Guardian subs |
| `0x3F` | Personal shop (subs `0x00`–`0x13`) |
| `0xAF` | `0x01` event match |
| `0xB1` | Map server change `0x00`–`0x01` |
| `0xB2` | Battle Castle block (large `switch`) |
| `0xB3`–`0xB6`, `0xBB` | BC / guild NPC locations |
| `0xB7`–`0xB9` | Catapult / kill count / castle hunt |
| `0xBA` | `ReceiveSkillCount` |
| `0xBD` | Crywolf (`0x00`–`0x0C`, …) |
| `0xC0`–`0xCB`, `0xCA` | Friends / letters / chat room |
| `0x2D` | `ReceiveBuffState` |
| `0xD1` | Kanturu 3rd / Raklion (`switch` subs) |
| `0xBF` | Cursed temple + lucky coin + Doppelganger (`0x00`–`0x14`, …) |
| `0xDE` | Character card (`0x00`) |
| **`0xD2`** | In-game shop (`#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM`) — many subs |
| **`0x4A` / `0x4B`** | Monk skills (`#ifdef PBG_ADD_NEWCHAR_MONK_SKILL`) |

If a **new** `case` is added to `TranslateProtocol`, append a row here (M1 maintenance).

---

## 3. Client TX — common `F3` subs (to server)

| Sub | Macro / API | Role |
|-----|-------------|------|
| `0x00` | `SendRequestCharactersList` | List characters |
| `0x01` | `SendRequestCreateCharacter` | Create |
| `0x02` | `SendRequestDeleteCharacter` | Delete |
| `0x03` | `SendRequestJoinMapServer` / Android enter | Join / enter map |
| `0x12` | `SendRequestFinishLoading` | Client finished loading zone |
| `0x06` | `SendRequestAddPoint` | Stat add (init `0xC1,0xF3`) |

Many non-`F3` TX heads are logged in `wsclientinline.h` (`0x19` magic, `0x24` move item, `0x32` buy, …).

---

## 4. `ProtocolCoreEx` overlay (custom / shared early dispatch)

Runs **before** `TranslateProtocol`. Examples from `Protocol.cpp` (non-exhaustive — depends on `#if`):

| Head | Notes |
|------|--------|
| `0xF3` | Subs like `0x00` (buff reset + `ReceiveCharacterList`), `0x03` `GCCharacterInfoRecv`, `0xE0`/`0xE1` new character info/calc, `0xEE` chaos box, custom coin / ranking / off-trade subs |
| `0xF1` sub `0x00` | `GCConnectClientRecv` |
| `0x11`, `0x16`, `0x17`, `0x26`, `0x27` | Damage / die / life / mana shortcuts |
| `0x2D`, `0x3F`, `0x4E`, `0x78`, `0xB1`, … | Buffs, personal shop hooks, post items, map move buff reset, … |

**Parity implication:** a minimalist `server-next` game host must not assume **only** `TranslateProtocol` — QA builds with extra `#define`s may need the same subs as production Takumi.

---

## 5. Next steps (handoff to M2+)

| Milestone | Action |
|-----------|--------|
| **M2** | ~~Move join / inventory / list builders from `LegacyLoginHost/Program.cs` into `Takumi.Server.Protocol`~~ **Done** (`CharacterListWire602`, `JoinMapServerWire602`, …; tests in `Takumi.Server.Tests`). |
| **M6** | For each **must-have** world packet, map a row from §2 → C# handler + test vector. |
| **Golden pcap** | Label captures with **client build flags** and **Dec2** identity. |

---

*Revision: initial M1 inventory from repo paths above. Extend when `TranslateProtocol` / `ProtocolCoreEx` / `LegacyLoginHost` changes.*
