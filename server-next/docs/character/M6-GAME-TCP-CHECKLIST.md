# M6 — Dedicated game TCP (`Takumi.Server.Game` + `Takumi.Server.GameHost`)

**Iteration status:** **COMPLETE** (2026-05-15) — split-stack **minimal-login** on `game-host`, Docker profile **`gamehost`**, real-device Android smoke from Connect **44605** through **F4 06 → F4 03 → game TCP** to **Main Scene**.

Last updated: 2026-05-17 — shop/inventory wire + level-up VFX; session logs: **`../../docs/journal/DEVELOPMENT-LOG-2026-05-17.md`**, **`../../docs/journal/DEVELOPMENT-LOG-2026-05-16.md`** (combat/FPS baseline).

**2026-05-15 baseline:** Android RX order **SM+XOR then strip `gProtect`**; `[event=decrypted_rx]`; APK resets SM serial per `Connect`; `docker-stack.sh` force-recreates `legacy-login` + `game-host`.

## Goal

Mirror **`Source/4.GameServer`** after client TCP connect: **`GCConnectClientSend`** → plain **`C1 F1 00`**, then the same **SimpleModulus + XOR32** pipeline as **`LegacyLoginHost`**.

When **`TAKUMI_GAME_PORT`** differs from **`TAKUMI_LOGIN_PORT`**, the client follows ConnectServer **`C1 F4 03`** to this port and must be able to **log in again**, see the **character list**, and **enter the world** (`F3 03`) using the same **`takumi-roster/<account>.json`** files as the login host.

### Troubleshooting: disconnect / “mất kết nối” right after choosing a sub-server

- Android log: **`connect failed … port=55901 errno=111 (Connection refused)`** means nothing is **listening** on that host:port.
- **Cause:** `.env` sets **`TAKUMI_GAME_PORT`** to a dedicated port, but only **`legacy-login`** is running (it still listens on **`TAKUMI_LOGIN_PORT`**, e.g. 44606). F4 03 advertised 55901 → client TCP to 55901 → refused.
- **Fix (pick one):**
  1. **Single TCP (simplest):** remove **`TAKUMI_GAME_PORT`** / **`TAKUMI_GAME_PUBLISH`** from `.env` (or leave them unset). F4 03 then defaults to **`TAKUMI_LOGIN_PORT`**.
  2. **Split stack:** keep **`TAKUMI_GAME_PORT=55901`** and start **`GameHost`** — e.g. `./scripts/docker/docker-stack.sh --detach` (auto-enables **`gamehost`** when `TAKUMI_GAME_PORT` is a positive integer), or `docker compose --profile gamehost up -d`, or `./scripts/docker/docker-stack.sh --detach --with-gamehost` if the port is unset but you still want the container.

## Modes (`GameListenHost`)

| Mode | When | Behaviour |
|------|------|-----------|
| **minimal-login** | `TAKUMI_ACCOUNTS` resolves to a non-empty map **and** `TAKUMI_SERVER_SERIAL` is 16 ASCII bytes | `F1 01` login, auto `F3 00` list (unless `TAKUMI_SKIP_AUTO_CHARLIST=1`), **`F3 01` / `F3 02`** create/delete, `F3 03`/`F3 15` join + **`F3 10`**, move-map **`8E 02`** → gate spawn + **`8E 03`** + **`0x1C`** + **`F3 03`/`F3 10`** (`MoveMapCatalog` / `Move.txt`), walk/instant-move roster updates, keepalive **`C1 03 71`**, `F1 02` logout ack |
| **bootstrap-only** | Accounts missing or serial invalid | Join + decrypt RX log only (`TAKUMI_VERBOSE=1`) |

`RepoEnvLoader` loads **`env.defaults`** then **`.env`**. If `.env` omits accounts/serial, **`env.defaults`** still supplies them when you run from `server-next/` (same as LegacyLoginHost).

## Exit criteria (this iteration) — all satisfied

These items are **frozen** for the 2026-05-15 M6 close; new scope belongs in a follow-up milestone (e.g. deeper world sync, optional §4 wire ticket QA).

- [x] `Takumi.Server.GameHost` binds **`TAKUMI_GAME_PORT`**.
- [x] **minimal-login** path: roster JSON via **`GameRosterDisk`** (same layout as LegacyLoginHost).
- [x] Docker **`--profile gamehost`** service **`game-host`**.
- [x] Host **`.env`** example: `TAKUMI_GAME_PORT` + `TAKUMI_GAME_PUBLISH` aligned with F4 03.
- [x] **minimal-login** parity vs single TCP: **`GamePortKeepAliveRunner`** (env `TAKUMI_GAME_KEEPALIVE_SECONDS`) + **`F3 01` / `F3 02`** create/delete character (shared **`GamePacketFinders`** / **`GameRosterMutations`**).
- [x] **M6+ (partial):** dòng log **`[event=decrypted_rx]`** trên stderr khi `TAKUMI_VERBOSE` hoặc **`TAKUMI_STRUCTURED_LOG=1`**; **`GamePortMinimalSession`** upsert **`character_roster`** qua **`CharacterRosterMirrorWriter`** khi `TAKUMI_ROSTER_DB_SYNC` (cùng `SaveRoster` như Legacy).
- [x] Cross-process **session ticket** on client wire (**HMAC-SHA256**, `Takumi.Server.Protocol` **`SessionTicketWire602` / `SessionTicketSignature602`**): **LegacyLoginHost** pushes **`C1 F1 A5` + 66 bytes** after successful F1 01 when **`TAKUMI_SESSION_HANDOFF_DB`** + **`TAKUMI_SESSION_TICKET_HMAC_KEY`** (UTF-8, ≥8 bytes) are set; **GameHost** optional **`TAKUMI_GAME_TICKET_WIRE=1`** → client must send **`F1 0xA6`** + same 66 bytes (encrypted like other client packets) before **`F1 01`**; **`PostgresSessionHandoffRepository.TryConsumeSignedWireAttachAsync`** consumes the row on attach. **`TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF`** (IP consume on F1 01) is **skipped** when wire mode is on (ticket already consumed on A6). Source client: **`ReceiveServerNextSessionTicket`**, **`Takumi_SendSessionTicketAttachIfPending`** in **`SendRequestLogIn`** (`WSclient.cpp` / `wsclientinline.h`).
- [x] `F3 10` after join / move-map: **`JoinInventoryPacket602`** (same as LegacyLoginHost) — empty when DB sync off; loads **`inventory_slot`** when **`TAKUMI_ROSTER_DB_SYNC`** + table present (`sql/init/002_inventory_slot.sql`).

## Sign-off — verified smoke (2026-05-15)

Use this table when closing a QA round; values assume default LAN ports (**44605** Connect, **55901** game publish).

| Gate | Android (`TakumiErrorReport` / `MuPreload`) | Host (`legacy-login` / `game-host`) |
|------|---------------------------------------------|--------------------------------------|
| F4 06 | `parsed … F4 06`, `Success Receive Server List` | `[connect] recv … F406` → `sent … ServerList (135 bytes)` |
| F4 03 | `ReceiveServerConnect` → `TCP session start … port=55901` | `[connect] recv … F403` → `ServerInfo ip=<LAN> port=55901` |
| Game join | `ReceiveJoinServer result=0x01`, `Post-CS redirect … gamePort=55901` | `listening on *:55901`, `sent join C1 F1 00`, `m6_minimal_session_begin`, `protect_inbound_pump on` |
| Login / char / world | `Try to Login`, `ReceiveList`, `Character scene`, `Main Scene init success` | `login ok id=…`, `[event=decrypted_rx]`, `F3 00`, `sent … F3 03` + `F3 10` |
| In-world combat | Melee `0x11`, kill EXP, level-up (FLARE VFX client-side) | `[m9] combat hit`, `[m7-exp]`, `sent join` + `F3 E1` / vitals |
| Shop / inventory (2026-05-17) | NPC shop → `F3 E9` prices; buy + inv drag | `[m8] shop buy`, `F3 10` resync, `0x24` move |

**Stack command (recommended):** `./scripts/docker/docker-stack.sh --host-build --recreate --detach` from `server-next/` — host `dotnet build` sanity check, pull, up, **force-recreates** `legacy-login` and `game-host`. Wait for **`[legacy-login] build OK`** (hoặc `TAKUMI_SKIP_CONTAINER_BUILD=1` + host-built DLL). Optional: set **`TAKUMI_SKIP_CONTAINER_BUILD=1`** in `.env` when dùng `--host-build` để container `dotnet exec` IL thay vì build lại trong Linux VM.

**Compose profiles:** if you run plain `docker compose up -d` in a shell **without** `COMPOSE_PROFILES`, services under profiles **`datazip`** / **`gamehost`** may not start. Either add to **`.env`**: `COMPOSE_PROFILES=datazip,gamehost`, or always use **`docker-stack.sh`** (it merges profiles when `TAKUMI_GAME_PORT` > 0 and datazip is on by default).

**Optional follow-up (not part of this M6 close):** Postgres wire ticket **`F1 A5` / `F1 A6`** (§4) — tick separately after `TAKUMI_SESSION_HANDOFF_DB` + HMAC + client verification.

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

If a local **`.env`** contains **`TAKUMI_PUBLISHED_SERVERS`** (or similar ad-hoc keys), note that **`server-next` does not read that variable** — Connect sub-server rows come from the built-in **F4 06** builder / BMD alignment (`protocol/M3-CONNECT-BMD.md`, **`TAKUMI_CS_CONNECT_*`**). Remove unused keys to avoid confusion.

## Run (two terminals, host)

Terminal A — login + connect (when **`TAKUMI_GAME_PORT`** is set, F4 03 points at that port — then Terminal B must run):

```bash
cd server-next
./scripts/host/run-legacy-login-host.sh
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
cd server-next
# Recommended: pull + up + force-recreate legacy-login + game-host (profiles merged from .env / script)
./scripts/docker/docker-stack.sh --detach
# Follow logs (same services the script tails when run without --detach):
docker compose logs -f legacy-login game-host postgres datazip
```

Explicit compose (no script):

```bash
docker compose --profile datazip --profile gamehost up -d --pull always --remove-orphans
docker compose up -d --force-recreate --no-deps legacy-login game-host
```

If **`TAKUMI_GAME_PORT`** is **unset** but you still want **`game-host`** for experiments: `./scripts/docker/docker-stack.sh --detach --with-gamehost`.

## QA Android (thiết bị thật) — checklist từng bước

Mục tiêu: xác nhận **split port** (Connect → F4 03 → game TCP), **minimal-login** trên `GameHost`, và (tuỳ chọn) **ticket ký trên wire** (`F1 A5` / `F1 A6`).

### Checklist nhanh (M6 iteration — đã tick khi smoke 2026-05-15)

- [x] **0** — Host `.env`: `TAKUMI_PUBLIC_HOST`, ports, **`TAKUMI_GAME_PORT` + `TAKUMI_GAME_PUBLISH`** khớp F4 03; `Dec2.dat` khớp client.
- [x] **1** — APK build + cài máy thật; bootstrap server trỏ cùng LAN host.
- [x] **2** — `watch-android-takumi-log.sh` (hoặc logcat tương đương) bắt được luồng TCP.
- [x] **3B** — Split: sau F4 03, **`TCP session start port=<TAKUMI_GAME_PORT>`**, không `errno=111`; có **`Success Receive Server List`** trước đó.
- [x] **3B suite** — `ReceiveJoinServer`, login, char list, chọn nhân vật, **`Main Scene init success`**.
- [ ] **3A** — Single-TCP path (`TAKUMI_GAME_PORT` trống, chỉ `legacy-login`) — regression tùy chọn, không bắt buộc để đóng M6 split.
- [ ] **§4** — Wire session ticket `F1 A5` / `F1 A6` + Postgres — QA nâng cao, không bắt buộc cho close iteration này.

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
./scripts/android/watch-android-takumi-log.sh
```

- Script gọi `adb logcat -c` trước (mặc định). **Sau khi chạy script, mở app / login lại** — nếu không có dòng nào là bình thường khi chưa tương tác.
- Giữ buffer cũ: `TAKUMI_LOGCAT_CLEAR=0 ./scripts/android/watch-android-takumi-log.sh`
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
   Chưa có process lắng nghe đúng cổng F4 03. Chạy `./scripts/docker/docker-stack.sh --detach` (hoặc `docker compose --profile gamehost up -d`, hoặc `./scripts/docker-up.sh --with-gamehost`). Nếu chỉ `legacy-login` mà `.env` vẫn set `TAKUMI_GAME_PORT` khác `TAKUMI_LOGIN_PORT` → client sẽ bị refused sau chọn server.

2. **Kết nối game OK nhưng sau `> Try to Login` không chuyển màn**  
   - Trên **host** (tên **service** trong compose là `game-host`, không phải tên container):  
     `docker compose --profile gamehost logs game-host 2>&1 | rg 'mode=|sent join|login ok|login rejected'`  
     Hoặc theo container: `docker logs takumi-game-host …`  
   - Cần thấy **`mode=minimal-login`** (có `TAKUMI_ACCOUNTS` + serial 16 byte). Nếu chỉ **`mode=bootstrap-only`**, server vẫn gửi join nhưng **không xử lý / không trả lời F1 01** → client đứng yên.  
   - Container `game-host` trong `docker-compose.yml` đã truyền sẵn `TAKUMI_ACCOUNTS` / `TAKUMI_SERVER_SERIAL` (cùng giá trị mặc định với `env.defaults`). Nếu bạn override sai (serial ≠ client, account không khớp), log sẽ có `login rejected: …`.  
   - **Đã sửa (2026-05-15):** heuristic `F3 00` không còn `return` sớm trên gói login C3 lớn (~90 byte) — trước đây có thể khớp nhầm và bỏ qua `F1 01` trên `GamePortMinimalSession` / `LegacyLoginHost` (xem `GamePacketFinders.TryFindCharacterListRequest`).

2b. **Logcat `packet sync lost` / byte đầu không phải `C1` ngay sau `recv tcp` join (split cổng game)**  
   Trên Android `ENCRYPT_STATE=1`, mọi recv trên cổng nằm trong `GSPortMin..GSPortMax` (vd. **55901**) chạy **`gProtect.DecryptData`** trước khi parse (`android_link_stubs.cpp`). **`GameHost`** mặc định XOR toàn bộ outbound cho khớp (log boot: `client protect wire … on`). Nếu tắt lớp này: **`TAKUMI_GAME_CLIENT_PROTECT_WIRE=0`**. Nếu vẫn rác: đối chiếu **`TAKUMI_SERVER_SERIAL`** + **`TAKUMI_PROTECT_CUSTOMER_NAME`** với client (`InitializeTakumiProtectState` / `CBGetMain.bin`).

   **Chiều vào server (RX) trên cổng GS (55901+):** `SendPacket(..., TRUE)` tạo gói **SimpleModulus `C3`** trước; sau đó `CWsctlc::sSend` gọi **`gProtect.EncryptData` trên toàn bộ buffer gửi đi** (ngoài SM), rồi `RawSend` vì `ConnectionManager_Connect(..., isEncrypted=0)`. Thứ tự trên dây là **`gProtect( SM(C3) )`**, không phải SM bọc gProtect. **`GameHost`** phải chạy **`TakumiClientProtectInboundPump`** (gỡ gProtect theo chunk TCP) **trước** `PipelinedDecryptor` (SM rồi Xor32). Nếu gỡ gProtect *sau* SM hoặc bỏ pump → decryptor không nhận ra `C3` → không có `[event=decrypted_rx]` / `login ok`.

   **Bộ đếm SM (Android, 2026-05-15):** Windows `CreateConnectSocket` reset `g_byPacketSerialSend` / `g_byPacketSerialRecv` sau **mỗi** `Connect` thành công; Android trước đây không reset → TCP mới tới `GameHost` có thể gửi `C3` với serial lệch so với `PipelinedDecryptor` (counter = 0) → host đóng phiên / không xử lý login. **Cần APK mới** (`android_link_stubs.cpp`: reset sau `ConnectionManager_Connect`). Trên log `game-host` bản mới: dòng **`protect_inbound_pump on`** khi có client protect wire.

3. **Sau `TCP session start` cổng game mà logcat không có `[AndroidLogin] recv tcp` trong vài giây**  
   Thường là **chưa có byte từ server** (firewall/NAT hiếm) hoặc cần đối chiếu với log host xem có dòng **`sent join C1 F1 00`** ngay khi phone connect không. Nếu host đã `sent join` mà phone không `recv tcp`, kiểm tra đúng IP LAN và cổng publish (`TAKUMI_GAME_PUBLISH`).

4. **Bật `TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF` / `TAKUMI_GAME_TICKET_WIRE` mà chưa cấu hình Postgres + ticket**  
   F1 01 có thể bị từ chối im lặng hoặc `login rejected` trên console GameHost — xem mục §4 bên dưới.

5. **Crash `pthread_mutex_lock` on destroyed mutex** sau lỗi kết nối  
   Tách bug teardown client; sau khi sửa lỗi TCP (111 / sai port), cài lại APK và thử lại.

6. **`game-host` build trong Docker: NETSDK1064, MSB1001 `-c`, hoặc legacy-login mất `project.assets.json`**  
   - **NETSDK1064:** bind-mount `/app` + `obj/` từ **macOS/Windows** → đường dẫn NuGet không khớp **Linux**. Dịch vụ `game-host` đặt **`TAKUMI_DOCKER_GAMEHOST=1`**: `Directory.Build.props` chuyển toàn bộ graph GameHost sang **`/tmp/takumi-gamehost/{obj,bin}/…`** (không đụng `src/*/obj` trên volume).  
   - **MSB1001 / `Switch: -c`:** `dotnet restore` **không** nhận `-c Release` — chỉ `dotnet build`/`run` mới dùng `-c`.  
   - **Không** xóa hàng loạt `src/Takumi.Server.*/obj` khi **`legacy-login`** cũng bind cùng volume: sẽ gây **NETSDK1004** / **CS2001** / **CS2012** trên container kia.

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

### F3 10 inventory resync sau join (2026-05-18)

- [x] Server: `GamePortMinimalSession` gửi **`F3 03` + `F3 10`** sau character select (`name='mg001'` trong log host).
- [x] Client: `ReceiveInventory` → `SetCharacterClass(Hero)` + `CreatePetDarkSpirit_Now` — refresh mesh từ equipment, không chỉ UI grid.
- [x] Android log: `[ReceiveInventory] slot=N type=… canEquip=…` + `[TakumiWear] weapon0/wing … meshs=…` — xem **`../../docs/journal/DEVELOPMENT-LOG-2026-05-18.md`**.
- [ ] Device QA: MG cánh **Lôi Vũ (6183)** + kiếm 58 `meshs>0` sau rebuild APK.

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
| `TAKUMI_GAME_CLIENT_PROTECT_WIRE` | Default **1** in Docker `game-host`: XOR outbound to match Android **Protect** on game TCP; set **`0`** only if the client build has no Protect layer on GS ports. |
| `TAKUMI_GAME_DUMP_PROTECT_IN` | `1` / `true`: verbose dump around inbound Protect strip (debug only). |
| `COMPOSE_PROFILES` | Optional in **`.env`**: e.g. `datazip,gamehost` so a bare `docker compose up -d` starts the same profile set as `docker-stack.sh`. |
