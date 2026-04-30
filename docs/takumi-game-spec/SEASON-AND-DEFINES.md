# Season & preprocessor — Takumi server (EXE603 baseline)

Tham chiếu rà codebase **2026-04-30**. Build production hiện tại (xuất vào `MuServer` trong `.vcxproj`) dùng cấu hình **`*_UPDATE=603`**.

## Các project MSVC — định nghĩa chính

| Project | Macro (release 603 chính) | Ý nghĩa gợi ý |
|---------|---------------------------|----------------|
| `ConnectServer.vcxproj` | `CONNECTSERVER_UPDATE=603` | EX603 lineage cho Connect |
| `DataServer.vcxproj` | `DATASERVER_UPDATE=603` | EX603 lineage cho Data |
| `JoinServer.vcxproj` | `JOINSERVER_UPDATE=603` | EX603 lineage cho Join |
| `GameServer.vcxproj` | `GAMESERVER_UPDATE=603`; `GAMESERVER_TYPE` 0 hoặc 1; `GAMESERVER_LANGUAGE=1` | `TYPE=0` → `GS`/`GameServer.exe`; `TYPE=1` → `GSCS`/`GameServerCS.exe` |
| `GetMainInfo.vcxproj` | Chỉ `WIN32;_CONSOLE` (Release) | Không ép season qua UPDATE macro trong project |

**Cấu hình khác trong cùng solution:** các target `*_401`, `*_803`, `*_CS` — dùng khi build nhiều season; không phải default output vào `MuServer` của snapshot checklist (xem `.vcxproj` `OutDir`).

### GameServer `OutDir` (hai shard trong batch)

- `Release_EX603` (**non-CS**) → `MuServer\4.GameServer\Sub 1\GameServer` → `GameServer.exe`
- `Release_EX603CS` → `MuServer\4.GameServer\GameServer` + `TargetName=GameServerCS` → `GameServerCS.exe`

## `GameServer/stdafx.h` (mặc định khi không override từ vcxproj)

- `GAMESERVER_UPDATE`: default **803** trong header nếu không set — build 603 ép từ `PreprocessorDefinitions`.
- Chuỗi season theo macro: **401 → SEASON 4**, **603 → SEASON 6**, **803 → SEASON 8**.
- `GAMESERVER_TYPE`: 0 GS / 1 GSCS (Castle Siege build).
- `PROTECT_STATE` / `ENCRYPT_STATE`: thường `1` (Premium + nhánh encrypt/logic Protect).
- Rất nhiều cờ `CUSTOM_*` — mở rộng parity: grep trong `Source/4.GameServer/GameServer/stdafx.h`.

## Coupling `Source/Util/` (server — theo `.vcxproj`)

| Tiện ích | Connect | Data | Join | Game |
|----------|---------|------|------|------|
| `..\..\Util\*` compiled | không | không | `MD5.cpp` + headers | `CCRC32.Cpp`, `Math.cpp` + headers |

Thư **`cryptopp` / `detours` / `lua` / `mapm`**: không có `ClCompile`/`ClInclude` từ bốn project server trong snapshot `.vcxproj`; backlog nếu cần grep thư viện tĩnh trong source.
