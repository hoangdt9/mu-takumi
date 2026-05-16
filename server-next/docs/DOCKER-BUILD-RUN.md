# Docker — build & run (Takumi `server-next`)

Last updated: 2026-05-16

Hướng dẫn **một chỗ** cho QA Android / dev LAN: chạy Postgres + server .NET trong Docker, biết **khi nào cần làm gì** (không nhầm *build image* với *build C# trong container*).

**Liên quan:** `../README.md`, `../docker-compose.yml`, `../scripts/docker-stack.sh`, `../../docs/ANDROID-DEV-MAC.md`, `M6-GAME-TCP-CHECKLIST.md`.

---

## 1. Hiểu nhanh (quan trọng)

Stack `server-next` **không** bake code C# vào Docker image.

| Thành phần | Cách hoạt động |
|------------|----------------|
| **Image Docker** | Chỉ image công khai: `postgres:16-alpine`, `mcr.microsoft.com/dotnet/sdk:10.0`, `nginx:alpine`. **Không có** `Dockerfile` / `build:` trong compose. |
| **Code server** | Thư mục `server-next/` **bind-mount** vào container (`/app`). |
| **“Build” server** | Mỗi lần container `legacy-login` / `game-host` **khởi động lại**, entrypoint chạy `dotnet build` + `dotnet run` **bên trong** container (~1–3 phút). |
| **Lệnh hàng ngày** | `./scripts/docker-stack.sh` — pull image + `up` + **force-recreate** `legacy-login` (và `game-host` nếu bật). |

```text
  Máy dev (git pull, sửa .cs)
        │
        ▼ bind-mount .:/app
  ┌─────────────────────────────┐
  │  container dotnet/sdk:10    │
  │  dotnet build → dotnet run    │  ← đây là “rebuild server”, KHÔNG phải docker build
  └─────────────────────────────┘
        │
        ▼
  Điện thoại APK → LAN IP :44605 / :44606 / :55901 …
```

**Kết luận:** Sau khi sửa C# hoặc `git pull`, thường chỉ cần chạy lại `docker-stack.sh` — **không** cần `docker compose build`.

---

## 2. Chuẩn bị (lần đầu)

1. **Docker Desktop** đang chạy (Mac/Windows/Linux).
2. Trong `server-next/`:

   ```bash
   cp .env.lan.example .env
   ```

3. Sửa `.env`:
   - `TAKUMI_PUBLIC_HOST` = IP LAN máy (cùng Wi‑Fi với điện thoại), ví dụ `192.168.1.50`.
   - `TAKUMI_LAN_IP` = cùng IP (nếu doc/script yêu cầu).
   - `TAKUMI_DATA_ZIP_URL=http://<IP>:18080/data.zip` (nếu dùng profile **datazip**).
4. **`keys/Dec2.dat`** — cùng file với client `Data/Dec2.dat` (mount vào container tại `/keys/Dec2.dat`).
5. Tắt stack khác trùng cổng (vd. `takumi-openmu` **44505**) khi test `server-next` (**44605** / **44606**).

**Cổng mặc định** (override trong `.env`):

| Dịch vụ | Cổng host (mặc định) | Ghi chú |
|---------|----------------------|---------|
| Connect | **44605** | `TAKUMI_CONNECT_PUBLISH` |
| Login / game (một TCP) | **44606** | `TAKUMI_LEGACY_LOGIN_PUBLISH` |
| Postgres | **54444** | `TAKUMI_POSTGRES_PUBLISH_PORT` |
| `data.zip` (nginx) | **18080** | profile **datazip** |
| GameHost (M6, tùy chọn) | **55901** | `TAKUMI_GAME_PUBLISH` + `TAKUMI_GAME_PORT` |

Kiểm tra port: `./scripts/check-lan-connect-ports.sh` hoặc `./scripts/check-takumi-ports.sh`.

### 2.1 Wi‑Fi LAN — Android QA (checklist)

Phone và Mac **cùng Wi‑Fi**, **tắt VPN** trên điện thoại. IP trong `.env` phải là IP LAN **hiện tại** của Mac (`ifconfig | grep "inet "` trên en0/wlan0).

```bash
# 1) .env
cd server-next
cp -n .env.lan.example .env   # nếu chưa có
# Sửa: TAKUMI_PUBLIC_HOST=192.168.x.x  (IP Mac)
#      TAKUMI_DATA_ZIP_URL=http://192.168.x.x:18080/data.zip

# 2) Stack (nhanh: host build + skip compile trong container)
./scripts/docker-stack.sh --host-build --recreate --detach

# 3) Đợi log
docker compose logs legacy-login game-host | grep -E 'build OK|listening|connect\] listening'
# legacy-login: [legacy-login] build OK …
# game-host (nếu TAKUMI_GAME_PORT): [game-host] build OK — listening on *:55901

# 4) Port trên Mac
./scripts/check-lan-connect-ports.sh
./scripts/smoke-connect-from-host.sh 192.168.x.x 44605   # từ Mac → phải OK

# 5) APK (Gradle đọc server-next/.env)
cd ../Source/android
./gradlew :app:assembleRealDevicePreloadDefaultDebug \
  -PmuRequiredAbis=armeabi-v7a,arm64-v8a
adb install -r app/build/outputs/apk/realDevicePreloadDefault/debug/*.apk
```

**Logcat — connect thành công (LAN):**

| Có | Không (lỗi) |
|----|-------------|
| `[Connect to Server …] ip=192.168.x.x port=44605` | `errno=110 Connection timed out` |
| Recv server list / không còn `no C2 F4 06 yet` | `connect fallback: revealed … (no C2 F4 06 yet)` → UI list **offline** |

**Sau chọn server (split M6, `TAKUMI_GAME_PORT=55901`):**

| Có | Không |
|----|-------|
| TCP tới `55901`, server log `sent join C1 F1 00` | `errno=111` hoặc không có log `game-host` |
| `ReceiveJoinServer result=0x01` (logcat) | Chỉ `decrypted_rx len=12` rồi disconnect → xem **M6-GAME-TCP-CHECKLIST.md** |

**Đơn giản hóa M6 (một TCP):** comment `TAKUMI_GAME_PORT` / `TAKUMI_GAME_PUBLISH` trong `.env`, rồi:

```bash
./scripts/docker-stack.sh --no-gamehost --host-build --recreate --detach
```

F4 03 trả cổng **44606**; login/game cùng `legacy-login`.

**AP isolation / firewall:** phone không ping được Mac → dùng USB: mục **§10** (`adb-reverse`), APK `-PmuBootstrapAdbReverse=true`, `TAKUMI_PUBLIC_HOST=127.0.0.1`.

Chi tiết client: `../../docs/ANDROID-DEV-MAC.md`. Wire game: `M6-GAME-TCP-CHECKLIST.md`.

---

## 3. Lệnh chính: `docker-stack.sh`

Chạy từ `server-next/`:

```bash
./scripts/docker-stack.sh              # pull + up + recreate legacy-login (+ game-host) + tail log
./scripts/docker-stack.sh --detach     # giống trên, không bám log (khuyên dùng sau pull code)
```

### Script làm gì (theo thứ tự)

1. Load `.env` (+ merge profile **datazip** mặc định; **gamehost** nếu `TAKUMI_GAME_PORT` > 0).
2. `docker compose pull` — cập nhật image postgres / sdk / nginx nếu tag đổi trên registry.
3. `docker compose up -d --pull always --remove-orphans`.
4. **Mặc định** (không có `--recreate` toàn stack):
   - `docker compose up -d --force-recreate --no-deps legacy-login`
   - Nếu có profile **gamehost**: tương tự **game-host**

Bước 4 đảm bảo process .NET cũ không giữ PID — **code C# mới** được `dotnet build` lại khi container start.

### Các flag thường dùng

| Flag | Khi nào dùng |
|------|----------------|
| `--detach` | Chạy nền; xem log bằng `docker compose logs -f …` |
| `--no-datazip` | Không bật nginx `data.zip` (port 18080) |
| `--with-gamehost` | Bật container **game-host** dù chưa set `TAKUMI_GAME_PORT` |
| `--no-gamehost` | Tắt auto game-host dù `.env` có cổng game |
| `--recreate` | Force-recreate **cả** service lúc `up` (postgres, datazip, …) |
| `--host-build` | `dotnet build` trên Mac **trước** khi lên container (kiểm tra compile; không tạo image) |
| `--recreate` | Force-recreate **toàn** stack lúc `up` (postgres volume giữ nguyên trừ khi xóa volume) |

**`--host-build` + startup nhanh:** trong `.env` đặt `TAKUMI_SKIP_CONTAINER_BUILD=1` để container `legacy-login` / `game-host` chạy `dotnet exec` trên DLL build sẵn trên host (Release, Linux-compatible). Script vẫn chạy `dotnet build` trên Mac để bắt lỗi compile trước khi `up`. Log: `[legacy-login] TAKUMI_SKIP_CONTAINER_BUILD=1 — using host-built IL`.

**Ghi chú client Android:** sau `git pull` server + client, rebuild APK (`main` ≥ `57f37a3` cho FPS/combat fixes) — xem **`../../docs/DEVELOPMENT-LOG-2026-05-16.md`**.

### Đợi gì trước khi mở app?

Trong log `legacy-login` (và `game-host` nếu có):

```text
[legacy-login] build OK — exec LegacyLoginHost …
```

Trước dòng đó cổng có thể **chưa** trả lời — đợi ~30–120 giây.

```bash
docker compose logs -f legacy-login postgres datazip game-host
```

---

## 4. Khi nào cần làm gì? (ma trận quyết định)

| Bạn thay đổi | Rebuild Docker **image**? | Chạy lại stack? | Rebuild APK? | Tải lại `data.zip`? |
|--------------|---------------------------|-----------------|--------------|---------------------|
| Chỉ file `.cs` trong `server-next` | **Không** | **Có** — `docker-stack.sh` | Không | Không |
| `.env`, `env.defaults`, SQL mới | **Không** | **Có** — recreate `legacy-login`; SQL volume cũ → `apply-sql.sh` | Không* | Không |
| Đổi IP/URL trong `.env` (LAN) | **Không** | **Có** | **Có** nếu URL bake vào Gradle `BuildConfig` | Không (trừ khi đổi nội dung zip) |
| Client C++ `Source/5.Main` | **Không** | Không | **Có** | Chỉ nếu đổi bundle |
| Nội dung `docker/data-zip/host/data.zip` | **Không** | Không (nginx đọc volume) | Không | Xóa cache app hoặc đổi URL |
| Đổi tag image trong `docker-compose.yml` (vd. sdk 11) | **`docker compose pull`** (script đã gọi) | **Có** | Không | Không |
| Thêm `Dockerfile` + `build:` (tương lai) | **`docker compose build`** | **Có** | — | — |

\* APK: chỉ khi bootstrap IP/URL lấy từ `.env` lúc Gradle configure.

---

## 5. Không dùng `docker compose build` (hiện tại)

`docker-compose.yml` ghi rõ:

- **Không** có khối `build:` cho `legacy-login`.
- `docker compose build legacy-login` **không** cập nhật logic `Program.cs` — image vẫn là `dotnet/sdk` trống.

**“Build server”** = log `[legacy-login] dotnet build` trong container sau **recreate**.

Nếu sau này team thêm image production riêng (CI), lúc đó mới dùng `docker compose build` — doc này sẽ cần bổ sung mục đó.

---

## 6. Profile Docker (compose)

| Profile | Service | Mục đích |
|---------|---------|----------|
| *(mặc định)* | `postgres`, `legacy-login` | DB + Connect **44605** + login/game **44606** (một process) |
| **datazip** | `datazip` (nginx) | `http://<LAN>:18080/data.zip` — `docker-stack.sh` bật mặc định |
| **gamehost** | `game-host` | TCP game riêng **M6** (`TAKUMI_GAME_PORT`, vd. **55901**) |
| **splitstack** | `connect-host`, `login-host` | Tách Connect / Login (không chạy cùng `legacy-login` — trùng cổng) |

**GameHost (Android F4 03 → cổng game):** trong `.env`:

```bash
TAKUMI_GAME_PORT=55901
TAKUMI_GAME_PUBLISH=55901
```

Rồi `./scripts/docker-stack.sh --detach` (script tự merge profile **gamehost**).

Hoặc trong `.env`: `COMPOSE_PROFILES=datazip,gamehost` nếu hay chạy `docker compose up` tay.

Chi tiết wire / QA: **`M6-GAME-TCP-CHECKLIST.md`**.

---

## 7. Lệnh thủ công (khi không dùng script)

```bash
cd server-next
docker compose pull
docker compose up -d
# Sau sửa C# — bắt buộc recreate để build lại:
docker compose up -d --force-recreate --no-deps legacy-login
docker compose up -d --force-recreate --no-deps game-host   # nếu profile gamehost đang bật
```

Dừng stack (giữ volume DB):

```bash
docker compose down
```

Xóa cả dữ liệu Postgres (volume):

```bash
docker compose down -v
```

---

## 8. Chạy server trên máy host (không Docker)

Dùng khi debug nhanh / hot reload — **không** bind cùng cổng với container:

```bash
./scripts/run-legacy-login-host.sh    # dotnet watch LegacyLoginHost
```

Trước đó: `docker compose stop legacy-login` (hoặc stop cả stack).

Nếu Connect qua Docker NAT gây lỗi C2 trên Android, thử host listener (ghi trong output `docker-stack.sh`).

---

## 9. Volume & dữ liệu game

| Mount (host → container) | Mục đích |
|--------------------------|----------|
| `.` → `/app` | Source C# |
| `./keys` → `/keys` | `Dec2.dat` |
| `../MuServer/4.GameServer/Data` → `/muserver-data` | `Monster.txt`, shop, gate (M8/M9) |
| `../docker/data-zip/host/Data` → `/att-data` | `Terrain.att` pathfinding |
| `../docker/data-zip/host` → nginx `/srv/data` | File `data.zip` |

Override trong `.env`: `TAKUMI_DEC2_HOST_DIR`, `TAKUMI_GAMESERVER_DATA_HOST`, `TAKUMI_ATT_DATA_HOST`, `TAKUMI_DATA_ZIP_HOST_DIR`.

**Postgres:** script `sql/init/*.sql` chỉ chạy **lần đầu** tạo volume. DB đã tồn tại:

```bash
./scripts/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime"
```

---

## 10. USB Android (không Wi‑Fi tới Mac)

```bash
./scripts/adb-reverse-takumi-dev.sh
```

Trong `.env` (và rebuild APK nếu bootstrap bake IP): `TAKUMI_PUBLIC_HOST=127.0.0.1`, `TAKUMI_LAN_IP=127.0.0.1`.

---

## 11. Xử lý sự cố

| Triệu chứng | Kiểm tra / xử lý |
|-------------|------------------|
| App không kết nối | `./scripts/check-lan-connect-ports.sh`; IP `.env` = IP Mac; tắt OpenMU trùng cổng |
| Login decrypt fail | `keys/Dec2.dat` trùng client; trong Docker **không** set `TAKUMI_DEC2_PATH` path macOS — compose dùng `/keys/Dec2.dat` |
| Sửa code mà hành vi cũ | Chưa recreate → `./scripts/docker-stack.sh --detach`; đợi `build OK` |
| `game-host` / M6 refused | `TAKUMI_GAME_PORT` khớp F4 03; container `game-host` đang chạy; log `[game-host] listening` |
| Không thấy quái/NPC | Mount `TAKUMI_GAMESERVER_DATA_HOST`; log `[m8]` / `[m9]`; env `TAKUMI_MONSTER_*` |
| Hai stack `datazip` | Không chạy `takumi/docker` datazip và `server-next` datazip cùng port **18080** |
| DB thiếu bảng | `./scripts/apply-sql.sh` (volume cũ) |

Smoke Connect từ Mac:

```bash
./scripts/smoke-connect-from-host.sh 127.0.0.1 44605
```

---

## 12. Tóm tắt một dòng

- **Hàng ngày (sửa server):** `cd server-next && ./scripts/docker-stack.sh --detach` → đợi `build OK` → test APK.
- **Không** `docker compose build` trừ khi compose có `build:` / Dockerfile mới.
- **Có** `docker compose pull` khi đổi version image trong compose.
- **Recreate** `legacy-login` / `game-host` = build & chạy lại C# từ repo đang checkout.
