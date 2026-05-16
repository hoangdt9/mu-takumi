# Development log — 2026-05-16

Ghi lại các hạng mục **đã merge trên `main`** (xem `git log` để chi tiết diff). Dùng kèm **`server-next/docs/IMPLEMENTATION-CHECKLIST.md`**.

**Tham chiếu commit (mới → cũ):**

| Commit | Tóm tắt |
|--------|---------|
| `57f37a3` | Sửa lỗi compile `ProtocolCompiler` (orphan `else`) sau packet budget |
| `13cc5f0` | Giảm FPS spike combat: batch level-up, cap gói/frame, throttle life `0x26` |
| `493bc09` | `SocketClient` connection check đúng kiểu object (native build) |
| `67c888b` | Docker LAN stack + Android M6 split login |
| `4e84ff3` | Combat stats HUD, stat-point flood, M7 persistence |
| `2672bfd` | EXP/level persist on kill + roster select |
| `3de80bb` | Chờ `F1 00` game-host trước login |
| `ec0b9ba` | Android mặc định LAN; `adb reverse` sau build flag |
| `e75cb12` | Mobile input, char create UI, login crash guards |
| `664b830` | Login, death FX, level-up từ kill EXP |
| `e428749` | Death/revival wire + vitals layout parity |
| `d8dfc84` | M7 stats, join vitals, Postgres SSOT |

---

## Server (`server-next`)

### M6 — Game TCP (split port)

- Profile **`gamehost`**: `TAKUMI_GAME_PORT` / `TAKUMI_GAME_PUBLISH` (vd. **55901**), F4 03 trỏ LAN IP.
- **`GamePortMinimalSession`**: minimal-login (F1 01, F3 00/01/02/03, F3 10, walk, keepalive `0x71`, gProtect inbound).
- **`./scripts/docker-stack.sh`**: pull, up, force-recreate `legacy-login` + `game-host`; flags `--detach`, `--host-build`, `--recreate`.
- Optional **`TAKUMI_SKIP_CONTAINER_BUILD=1`**: chạy IL build trên host (`dotnet exec` DLL trong container Linux).

### M7 — Character persistence & combat preview

- **`011_character_experience.sql`**: `experience` trên `character_roster` / `character_domain`.
- **`RosterExperienceCombat`**: grant EXP on kill, level-up server-side, mirror Postgres.
- **`CharacterStatPointHandler`**: xử lý **nhiều `C1 F3 06`** trong một TCP buffer → **một** `LevelUpPointWire602` success.
- **`NewCharacterCalcWire602`** (`C1 F3 E1`, 172 byte): gửi sau join trên Legacy + GameHost — HUD Damage/Defense/Speed (`GCNewCharacterCalcRecv`).
- **`CharacterCombatPreview602`**: preview từ sheet + level; monster→player defense không còn stub `level*3` thuần.

### M9 — Monster combat (in-world)

- Hit `C1 0x11` / skill, damage `0x11`, die `0x16`, destroy `0x14`, viewport `C2 0x13`.
- AI loop (~500ms): monster hit player → damage + life; có throttle outbound life (`TAKUMI_PLAYER_LIFE_PACKET_MIN_MS`, mặc định 400ms).

### M14 — Account Postgres

- `public.account`, login `F1 01`, register in-game `C1 D3 05` khi `TAKUMI_ACCOUNT_DB=1`.

---

## Android client (`Source/5.Main`)

### Login / M6

- Bootstrap LAN từ `server-next/.env` → Gradle `BuildConfig` (Connect **44605**, `data.zip` URL).
- Post-Connect: chờ **`F1 00`** game-host trước khi gửi login (`3de80bb`).
- Optional **`-PmuBootstrapAdbReverse=true`** + `adb-reverse-takumi-dev.sh` khi USB bị AP isolation.

### In-game combat & HUD

- **`TakumiSendMeleeAttack`**: gửi `C1 0x11` từ virtual pad trước pathfind/move (`android_main.cpp`).
- Nhận **`F3 E1`**: cập nhật panel chỉ số combat (không còn 0~0 khi chỉ có join packet).
- Kill EXP: `ReceiveDieExp` + level-up local; server mirror EXP qua DB.

### Stat points (crash / flood)

- Dialog cộng điểm: **`TakumiScheduleLevelUpPoints`** + pump tối đa **4 gói/frame** (`ZzzScene` / `TakumiPumpLevelUpPoints`).
- Debounce OK + dừng IME khi đóng dialog; server batch `F3 06` trong một buffer.

### Performance (2026-05-16)

- **Multi-level từ một kill**: `TakumiOnHeroLevelsGained(n)` — một lần effect + `CalculateAll` path thay vì N lần `TakumiPlayLevelUpEffects`.
- **`ProtocolCompiler`**: tối đa **20 gói/frame** trên Android; `CalculateAll` deferred sau `ReceiveAddPoint`.
- Attack repeat pad: **320ms**; cooldown gói melee **280ms**.

### QA đã smoke (thiết bị thật, log `TakumiErrorReport`)

- Connect → F4 06/03 → game **55901** → login `test1` → join `dw001` / `dk001`.
- Đánh quái: `[Combat] hit mob`, `mob died`, `[Exp] level up`.
- Server log: `[m9] combat hit`, `[m7-exp] level up`, viewport `0x13`.
- Còn mở: FPS dài khi đông quái (đã giảm); disconnect `invalid header` sau session dài; stat UI full QA.

---

## Build / chạy nhanh

```bash
# Server
cd server-next
cp .env.lan.example .env   # nếu chưa có
./scripts/docker-stack.sh --host-build --recreate --detach
# Đợi: [legacy-login] build OK

# APK
cd Source/android
./gradlew :app:assembleRealDevicePreloadDefaultDebug -PmuRequiredAbis=armeabi-v7a,arm64-v8a
adb install -r app/build/outputs/apk/realDevice/preloadDefault/debug/app-realDevice-preloadDefault-debug.apk
```

**Docs liên quan:** `server-next/docs/DOCKER-BUILD-RUN.md`, `server-next/docs/M6-GAME-TCP-CHECKLIST.md`, `server-next/docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`.
