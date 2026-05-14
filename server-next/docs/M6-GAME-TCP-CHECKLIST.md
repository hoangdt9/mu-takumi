# M6 — Dedicated game TCP (`Takumi.Server.Game` + `Takumi.Server.GameHost`)

Last updated: 2026-05-14 (M6 Android QA procedure; signed wire `F1 A5`/`F1 A6`; `[event=decrypted_rx]` + `TAKUMI_STRUCTURED_LOG`)

## Goal

Mirror **`Source/4.GameServer`** after client TCP connect: **`GCConnectClientSend`** → plain **`C1 F1 00`**, then the same **SimpleModulus + XOR32** pipeline as **`LegacyLoginHost`**.

When **`TAKUMI_GAME_PORT`** differs from **`TAKUMI_LOGIN_PORT`**, the client follows ConnectServer **`C1 F4 03`** to this port and must be able to **log in again**, see the **character list**, and **enter the world** (`F3 03`) using the same **`takumi-roster/<account>.json`** files as the login host.

### Troubleshooting: disconnect / “mất kết nối” right after choosing a sub-server

- Android log: **`connect failed … port=55901 errno=111 (Connection refused)`** means nothing is **listening** on that host:port.
- **Cause:** `.env` sets **`TAKUMI_GAME_PORT`** to a dedicated port, but only **`legacy-login`** is running (it still listens on **`TAKUMI_LOGIN_PORT`**, e.g. 44606). F4 03 advertised 55901 → client TCP to 55901 → refused.
- **Fix (pick one):**
  1. **Single TCP (simplest):** remove **`TAKUMI_GAME_PORT`** / **`TAKUMI_GAME_PUBLISH`** from `.env` (or leave them unset). F4 03 then defaults to **`TAKUMI_LOGIN_PORT`**.
  2. **Split stack:** keep **`TAKUMI_GAME_PORT=55901`** and start **`GameHost`** — e.g. `docker compose --profile gamehost up -d` or `./scripts/docker-stack.sh --detach --with-gamehost` — so something accepts TCP on the advertised port.

## Modes (`GameListenHost`)

| Mode | When | Behaviour |
|------|------|-----------|
| **minimal-login** | `TAKUMI_ACCOUNTS` resolves to a non-empty map **and** `TAKUMI_SERVER_SERIAL` is 16 ASCII bytes | `F1 01` login, auto `F3 00` list (unless `TAKUMI_SKIP_AUTO_CHARLIST=1`), **`F3 01` / `F3 02`** create/delete, `F3 03`/`F3 15` join + **`F3 10`**, stub move-map `8E 02` + `F3 03` + **`F3 10`**, walk/instant-move roster updates, keepalive **`C1 03 71`**, `F1 02` logout ack |
| **bootstrap-only** | Accounts missing or serial invalid | Join + decrypt RX log only (`TAKUMI_VERBOSE=1`) |

`RepoEnvLoader` loads **`env.defaults`** then **`.env`**. If `.env` omits accounts/serial, **`env.defaults`** still supplies them when you run from `server-next/` (same as LegacyLoginHost).

## Exit criteria (this iteration)

- [x] `Takumi.Server.GameHost` binds **`TAKUMI_GAME_PORT`**.
- [x] **minimal-login** path: roster JSON via **`GameRosterDisk`** (same layout as LegacyLoginHost).
- [x] Docker **`--profile gamehost`** service **`game-host`**.
- [x] Host **`.env`** example: `TAKUMI_GAME_PORT` + `TAKUMI_GAME_PUBLISH` aligned with F4 03.
- [x] **minimal-login** parity vs single TCP: **`GamePortKeepAliveRunner`** (env `TAKUMI_GAME_KEEPALIVE_SECONDS`) + **`F3 01` / `F3 02`** create/delete character (shared **`GamePacketFinders`** / **`GameRosterMutations`**).
- [x] **M6+ (partial):** dòng log **`[event=decrypted_rx]`** trên stderr khi `TAKUMI_VERBOSE` hoặc **`TAKUMI_STRUCTURED_LOG=1`**; **`GamePortMinimalSession`** upsert **`character_roster`** qua **`CharacterRosterMirrorWriter`** khi `TAKUMI_ROSTER_DB_SYNC` (cùng `SaveRoster` như Legacy).
- [x] Cross-process **session ticket** on client wire (**HMAC-SHA256**, `Takumi.Server.Protocol` **`SessionTicketWire602` / `SessionTicketSignature602`**): **LegacyLoginHost** pushes **`C1 F1 A5` + 66 bytes** after successful F1 01 when **`TAKUMI_SESSION_HANDOFF_DB`** + **`TAKUMI_SESSION_TICKET_HMAC_KEY`** (UTF-8, ≥8 bytes) are set; **GameHost** optional **`TAKUMI_GAME_TICKET_WIRE=1`** → client must send **`F1 0xA6`** + same 66 bytes (encrypted like other client packets) before **`F1 01`**; **`PostgresSessionHandoffRepository.TryConsumeSignedWireAttachAsync`** consumes the row on attach. **`TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF`** (IP consume on F1 01) is **skipped** when wire mode is on (ticket already consumed on A6). Source client: **`ReceiveServerNextSessionTicket`**, **`Takumi_SendSessionTicketAttachIfPending`** in **`SendRequestLogIn`** (`WSclient.cpp` / `wsclientinline.h`).
- [x] `F3 10` after join / move-map: **`JoinInventoryPacket602`** (same as LegacyLoginHost) — empty when DB sync off; loads **`inventory_slot`** when **`TAKUMI_ROSTER_DB_SYNC`** + table present (`sql/init/002_inventory_slot.sql`).

## `.env` (host QA)

**Default (only `legacy-login`):** do **not** set `TAKUMI_GAME_PORT`; omit it so F4 03 uses `TAKUMI_LOGIN_PORT`.

**Split port + Docker `gamehost`:** set both so F4 03 and the published container port match.

```bash
TAKUMI_PUBLIC_HOST=192.168.x.x
TAKUMI_CONNECT_PORT=44605
TAKUMI_LOGIN_PORT=44606
# Split M6 only — requires GameHost / `docker compose --profile gamehost`:
# TAKUMI_GAME_PORT=55901
# TAKUMI_GAME_PUBLISH=55901
```

Keep **`TAKUMI_DEC2_PATH`** (or `./keys/Dec2.dat` mount in Docker) identical to the Android client’s **`Data/Dec2.dat`**.

## Run (two terminals, host)

Terminal A — login + connect (when **`TAKUMI_GAME_PORT`** is set, F4 03 points at that port — then Terminal B must run):

```bash
cd server-next
./scripts/run-legacy-login-host.sh
# or: dotnet run --project src/Takumi.Server.LegacyLoginHost/...
```

Terminal B — game TCP:

```bash
cd server-next
dotnet run --project src/Takumi.Server.GameHost/Takumi.Server.GameHost.csproj -c Release
```

Smoke (listen): after start, `lsof -nP -iTCP:55901 -sTCP:LISTEN` should show **`Takumi.Server.GameHost`**.

## Run (Docker)

```bash
# .env must set TAKUMI_GAME_PORT=55901 (and legacy-login reads it for F4 03)
docker compose --profile gamehost up -d
# or: ./scripts/docker-stack.sh --detach --with-gamehost
```

## QA Android (thiết bị thật) — checklist từng bước

Mục tiêu: xác nhận **split port** (Connect → F4 03 → game TCP), **minimal-login** trên `GameHost`, và (tuỳ chọn) **ticket ký trên wire** (`F1 A5` / `F1 A6`).

### 0. Chuẩn bị host `.env` (cùng subnet với điện thoại)

- `TAKUMI_PUBLIC_HOST` = IP LAN máy chạy Docker/dotnet (ví dụ `192.168.1.50` — trùng Gradle `-PmuLanIp` / `.env` Android nếu có).
- `TAKUMI_CONNECT_PORT` / `TAKUMI_LOGIN_PORT` khớp client (`MU_BOOTSTRAP_SERVER` trong APK = `host:44605` là bình thường cho bootstrap).
- **Split M6:** bật `TAKUMI_GAME_PORT` + `TAKUMI_GAME_PUBLISH` (ví dụ `55901`) **và** phải có process lắng nghe đúng cổng đó (`GameHost` hoặc container `game-host`).
- `Dec2.dat` trên host (`TAKUMI_DEC2_PATH` hoặc `server-next/keys/Dec2.dat`) **trùng** bản trong `data.zip` / `Data/Dec2.dat` trên client.

### 1. Build & cài APK (đã làm trong log mẫu)

```bash
cd takumi/Source/android
./gradlew :app:assembleRealDevicePreloadDefaultDebug \
  -PmuRequiredAbis=armeabi-v7a,arm64-v8a \
  -PmuFailOnMissingRequiredAbis=true
adb install app/build/outputs/apk/realDevicePreloadDefault/debug/app-realDevice-preloadDefault-debug.apk
```

Gradle in ra `MU_BOOTSTRAP_SERVER` / `DATA_ZIP_URL_LAN` — phải trỏ tới **cùng host** đang chạy `server-next`.

### 2. Terminal logcat (`watch-android-takumi-log.sh`)

```bash
cd server-next
./scripts/watch-android-takumi-log.sh
```

- Script gọi `adb logcat -c` trước (mặc định). **Sau khi chạy script, mở app / login lại** — nếu không có dòng nào là bình thường khi chưa tương tác.
- Giữ buffer cũ: `TAKUMI_LOGCAT_CLEAR=0 ./scripts/watch-android-takumi-log.sh`
- Filter hiện tại: `MuPreload`, `MuMain`, `TakumiErrorReport`, `AndroidRuntime:E`. **Luồng TCP** (`[AndroidLogin] recv tcp`, `TCP session start`) thường nằm tag **`TakumiErrorReport`** (`AndroidNetwork.cpp`).
- **`g_ErrorReport.Write`** (ví dụ chuỗi `Received F1 A5` / `Sent F1 A6` trong `WSclient.cpp`) có thể **chỉ ghi file** trên Android, không qua `__android_log__` — nếu cần chắc chắn thấy ticket wire trên logcat, dùng tạm thời:
  - `adb logcat -v threadtime | rg 'AndroidLogin|F1 A5|F1 A6|recv tcp|session-ticket'`
  - hoặc kéo file log Takumi trên thiết bị (theo doc dự án `ANDROID-DEV-MAC.md` / `TakumiErrorReport`).

### 3. Hai lộ trình kiểm thử

**A — Single TCP (M6 tối thiểu, không bắt buộc GameHost):**  
Để **`TAKUMI_GAME_PORT` trống** → F4 03 dùng `TAKUMI_LOGIN_PORT`. Chỉ cần `legacy-login`. Trên logcat: sau chọn sub-server, `TCP session start port=<login_port>`, có `recv tcp` vài byte đầu = **`C1 F1 00`** (join).

**B — Split stack + GameHost (đúng nghĩa M6 hai cổng):**  
Bật `TAKUMI_GAME_PORT=55901`, chạy **`GameHost`** (hoặc Docker `--profile gamehost`). Trên logcat: lần kết nối sau F4 03 phải thấy **`TCP session start port=55901`** (hoặc đúng cổng bạn publish), **không** được `errno=111` (connection refused).

#### Treo ở màn nhập login sau khi chọn sub-server (split M6)

1. **`errno=111` trên cổng game (ví dụ 55901)**  
   Chưa có process lắng nghe đúng cổng F4 03. Chạy `docker compose --profile gamehost up -d` (hoặc `./scripts/docker-up.sh --with-gamehost`). Nếu chỉ `legacy-login` mà `.env` vẫn set `TAKUMI_GAME_PORT` khác `TAKUMI_LOGIN_PORT` → client sẽ bị refused sau chọn server.

2. **Kết nối game OK nhưng sau `> Try to Login` không chuyển màn**  
   - Trên **host** (tên **service** trong compose là `game-host`, không phải tên container):  
     `docker compose --profile gamehost logs game-host 2>&1 | rg 'mode=|sent join|login ok|login rejected'`  
     Hoặc theo container: `docker logs takumi-game-host …`  
   - Cần thấy **`mode=minimal-login`** (có `TAKUMI_ACCOUNTS` + serial 16 byte). Nếu chỉ **`mode=bootstrap-only`**, server vẫn gửi join nhưng **không xử lý / không trả lời F1 01** → client đứng yên.  
   - Container `game-host` trong `docker-compose.yml` đã truyền sẵn `TAKUMI_ACCOUNTS` / `TAKUMI_SERVER_SERIAL` (cùng giá trị mặc định với `env.defaults`). Nếu bạn override sai (serial ≠ client, account không khớp), log sẽ có `login rejected: …`.

3. **Sau `TCP session start` cổng game mà logcat không có `[AndroidLogin] recv tcp` trong vài giây**  
   Thường là **chưa có byte từ server** (firewall/NAT hiếm) hoặc cần đối chiếu với log host xem có dòng **`sent join C1 F1 00`** ngay khi phone connect không. Nếu host đã `sent join` mà phone không `recv tcp`, kiểm tra đúng IP LAN và cổng publish (`TAKUMI_GAME_PUBLISH`).

4. **Bật `TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF` / `TAKUMI_GAME_TICKET_WIRE` mà chưa cấu hình Postgres + ticket**  
   F1 01 có thể bị từ chối im lặng hoặc `login rejected` trên console GameHost — xem mục §4 bên dưới.

5. **Crash `pthread_mutex_lock` on destroyed mutex** sau lỗi kết nối  
   Tách bug teardown client; sau khi sửa lỗi TCP (111 / sai port), cài lại APK và thử lại.

### 4. Ticket ký trên wire (tuỳ chọn QA nâng cao)

Cần Postgres handoff + cùng một HMAC trên cả hai process:

- `TAKUMI_SESSION_HANDOFF_DB=1` + `TAKUMI_PG_*` + đã apply `sql/init/003_session_ticket.sql`.
- `TAKUMI_SESSION_TICKET_HMAC_KEY=<cùng một chuỗi UTF-8 ≥8 byte>` trên **cả** legacy-login **và** game-host.
- Trên **GameHost only:** `TAKUMI_GAME_TICKET_WIRE=1`.

**Host — Legacy:** sau login OK, console có dạng `sent F1 A5 session-ticket (wire) len=70`.  
**Host — Game:** khi client gửi attach, có `F1 A6 signed session-ticket verified …`; sau đó `login ok` trên `F1 01`.

**Client:** sau login legacy thành công, khi mở TCP game và gửi login, client gửi `F1 A6` trước `F1 01` (macro `SendRequestLogIn`). Xác minh tuỳ môi trường: logcat rộng hoặc file `ErrorReport`.

### 5. Lệnh host nhanh (không cần điện thoại)

```bash
cd server-next
dotnet test src/Takumi.Server.Tests/Takumi.Server.Tests.csproj -c Release
lsof -nP -iTCP:${TAKUMI_GAME_PORT:-55901} -sTCP:LISTEN   # sau khi start GameHost
```

**Lưu ý:** Agent không có `adb` tới máy của bạn; bước 1–4 bạn tự chạy trên máy LAN. Build `GameHost` + `LegacyLoginHost` trên repo: **Release build OK** (xác minh compile CI-local).

## Env reference

| Variable | Purpose |
|----------|---------|
| `TAKUMI_GAME_PORT` | **Omit** for single-TCP (F4 03 = login port). Set for **split** stack: must match GameHost listen port = F4 03 target. |
| `TAKUMI_GAME_PUBLISH` | Host port mapped into **`game-host`** (default **55901**). |
| `TAKUMI_ACCOUNTS` | Enables **minimal-login** when non-empty (with serial). |
| `TAKUMI_SERVER_SERIAL` | 16-byte ASCII; must match client. |
| `TAKUMI_JOIN_VERSION` | Same as login host. |
| `TAKUMI_GAME_JOIN_WIRE_INDEX` | Optional `ushort` for join packet index bytes (default **0**). |
| `TAKUMI_ROSTER_DIR` | Override roster JSON directory (default `./takumi-roster`). |
| `TAKUMI_STRUCTURED_LOG` | `1` / `true` = luôn ghi `[event=decrypted_rx]` trên stderr (minimal-login), kể khi không `TAKUMI_VERBOSE`. |
| `TAKUMI_ROSTER_PERIODIC_SAVE_SECONDS` | Khoảng thời gian (5–3600) flush roster JSON + Postgres khi có walk/instant move (M4c). |
| `TAKUMI_MAX_DECRYPTED_PACKET_BYTES` | Giới hạn độ dài một gói **sau decrypt** (mặc định **12288**); vượt quá → đóng TCP. |
| `TAKUMI_MAX_PACKETS_PER_SECOND` | **0** = tắt. Giá trị dương = cửa sổ 1 giây; vượt ngưỡng → đóng TCP (chống flood nhẹ). |
| `TAKUMI_SESSION_HANDOFF_DB` | `1` = ghi pending **`session_ticket`** sau F1 01 trên **LegacyLoginHost** (cần `003_session_ticket.sql` + `TAKUMI_PG_*`). |
| `TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF` | Trên **GameHost**: `1` = F1 01 chỉ OK sau khi **consume** một `session_ticket` (đăng nhập legacy trước). |
| `TAKUMI_SESSION_TICKET_HMAC_KEY` | Shared secret (UTF-8, ≥8 bytes) for **F1 A5/A6** ticket MAC; must match on **LegacyLoginHost** and **GameHost**. |
| `TAKUMI_GAME_TICKET_WIRE` | On **GameHost**: `1` = require **F1 A6** attach before **F1 01**; consumes **`session_ticket`** on attach (needs **`TAKUMI_SESSION_HANDOFF_DB`** + HMAC key). When on, **`TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF`** IP consume on F1 01 is not used. |
