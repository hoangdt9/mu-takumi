# Takumi Android — dev & test on macOS

Project: `Source/android` (Gradle + CMake + JNI).

## Prerequisites

- **JDK 17** (`java -version`).
- **Android SDK** (Android Studio → SDK Manager). Typical path: `~/Library/Android/sdk`.
- **`local.properties`** in `Source/android` must set `sdk.dir=...` (see repo file; mac template included).

## Fix Windows-only Gradle (if you cloned from PC)

Remove `org.gradle.java.home=...` from `gradle.properties` on Mac, or point it to Android Studio’s JBR, e.g.:

`/Applications/Android Studio.app/Contents/jbr/Contents/Home`

## Build APK (real phone — ARM)

From `Source/android`:

```bash
chmod +x ./gradlew
./gradlew :app:assembleRealDevicePreloadDefaultDebug \
  -PmuRequiredAbis=armeabi-v7a,arm64-v8a \
  -PmuFailOnMissingRequiredAbis=true
```

Optional: first-hop `data.zip` URL (Docker static host on LAN) — default `http://192.168.1.50:18080/data.zip`, override with `-PmuDataZipLan=http://YOUR_IP:18080/data.zip`.

Output: `app/build/outputs/apk/realDevicePreloadDefault/debug/` (APK name includes flavor segments; exact path may vary by AGP).

### Tránh tải lại `data.zip` mỗi lần rebuild (dev)

- **`assemble...Debug`** bật `BuildConfig.DEV_SKIP_DATA_ZIP` (mặc định **true**): nếu đã có file marker `.mu_data_ready_v1` hoặc thư mục `Data/` đủ heuristic (không bắt buộc đủ mọi file trong danh sách Takumi), PreloadActivity **bỏ qua** HTTP `data.zip`.
- Cài đè bằng `adb install -r` **cùng `applicationId`** (`preloadDefault`, không dùng `preloadDatafresh`) để giữ `Android/data/com.muonline.client/files/`.
- Buộc đồng bộ đủ như release: thêm `-PmuDevSkipDataZip=false` khi build debug.

## Build APK (emulator — x86)

```bash
./gradlew :app:assembleEmulatorPreloadDefaultDebug \
  -PmuRequiredAbis=x86,x86_64 \
  -PmuFailOnMissingRequiredAbis=true
```

## Install on device / emulator

```bash
adb install -r app/build/outputs/apk/realDevicePreloadDefault/debug/*.apk
```

APK buộc tải lại `data.zip` (package tách `.dataredl`): `assembleRealDevicePreloadDatafreshDebug` → thư mục output tương ứng `.../realDevicePreloadDatafresh/debug/`.

## Native libs & flavors

See `PORTING_TAKUMI_ANDROID.md` in `Source/android`. Prebuilt `.so` live under `app/src/main/jniLibs/<abi>/`.

## Optional flags

- `-PmuEnableLogs=true` — verbose native logs (see `app/build.gradle`).
- Android Studio: **File → Open** → folder `Source/android`, then **Build Variants** → choose `realDeviceDebug` / `emulatorDebug`.

## Chạy server trên Mac để test `android_main` (APK)

**Server trong `Source` (Connect/Data/Join/Game) là `.exe` Windows MSVC** — **macOS không chạy được**; Docker + Wine trên **Apple Silicon** thường lỗi. `android_main.cpp` chỉ là **client native**; để test vào game vẫn cần **máy chủ Windows** đâu đó có thể ping được từ điện thoại/emulator.

### Cách làm thực tế

1. **Chạy bộ `MuServer` trên Windows**
   - **Máy PC Windows** thật (LAN chung Wi‑Fi / cáp); hoặc
   - **VM Windows 11 ARM** trên Mac (**Parallels / UTM**) — copy **`takumi/MuServer`** vào guest, `Start_*.bat` Run as administrator, **đừng** dùng VM VMware x86 (`VMWare/BNS-2020`) trên Mac ARM.
2. **Gán IP / mở port** sao cho điện thoại hoặc emulator trỏ được tới CS/JS/GS (theo `Mu.ini` / `ServerInfo` trong `Data` của client).
3. **Build & cài APK** (lệnh phía trên), chỉnh IP server trong **`Source/5.Main/source/android_main.cpp`** / `GameConfig*` / `LauncherHelper` hoặc trong `Data` nạp theo `PreloadActivity` — khớp máy chủ bạn vừa bật.

### Emulator trên Mac

- Emulator **không** “thấy” `127.0.0.1` của Mac như server; dùng **`10.0.2.2`** chỉ khi server chạy **trên chính máy host** — mà host Mac **không chạy được** Win32 server. → Cần IP **VM Windows** hoặc **PC Windows** trong mạng (`ipconfig` trong Windows / `ifconfig` trong Mac).

### Log native khi test `android_main`

```bash
adb logcat -s MuMain MuPreload SDL
```

Chi tiết hơn (kèm ẩn mọi tag khác — **trên zsh phải bọc `'*:S'`** nếu không shell báo `no matches`):

```bash
adb logcat -c && adb logcat -v threadtime MuPreload:I MuMain:I TakumiErrorReport:I AndroidRuntime:E libc:F '*:S'
```

**Tóm:** Trên Mac chỉ **build Android**; server test = **Windows (máy hoặc VM)** + **mạng** + **IP client đúng**.

---

## Test Android với **Takumi Server Next** trên Mac (Docker tối thiểu)

Khi client/APK trỏ tới **Connect/Login Takumi** (ví dụ cổng **`44605` / `44606`** trong `server-next/.env`, biến `TAKUMI_PUBLIC_HOST` = IP LAN máy Mac), chỉ cần chạy **một** stack server — tránh chạy song song **OpenMU** (`44505`/`55901`…) cùng lúc nếu không cần, để log và cấu hình client không lẫn cổng.

### Docker Desktop: `server-next` stack (Postgres + LegacyLoginHost + tuỳ chọn data.zip)

- **`server-next/docker-compose.yml`** chạy **Postgres** (host **54444**) và **LegacyLoginHost** (Connect **44605** + login **44606**) trong Docker. `lsof` có thể hiện **`com.docke`** — bình thường.
- **Tuỳ chọn `data.zip` qua LAN:** cùng file compose, profile **`datazip`** — nginx publish cổng host **mặc định 18080** (khác **44605/44606/54444**). File đặt tại `takumi/docker/data-zip/host/data.zip`. Bật: `docker compose --profile datazip up -d` hoặc `./scripts/docker-up.sh --with-datazip`, hoặc `COMPOSE_PROFILES=datazip` trong `.env`. **Không** chạy đồng thời `datazip` của `server-next` và `takumi/docker` nếu cùng publish port **18080**.
- Container **`takumi-next-host`** (nếu còn trong máy bạn) là compose **cũ/riêng** — có thể xóa orphan hoặc `docker rm` nếu không dùng.
- **Đừng `docker restart takumi-next-host` nhẹ nhàng** nếu CMD đang là `dotnet run --project src/Takumi.Server.Host` mà repo **chưa có** project Host đầy đủ: container sẽ **crash loop** (`Couldn't find a project to run`). Cần **sửa Dockerfile/CMD** hoặc mount đúng source trước khi restart.
- Log kiểu **`[login] ignored unsupported MU packet`** trong container = server nhận **C3** nhưng **không xử lý** (decrypt/ghi không khớp hoặc handler thiếu) → client không có byte trả về.
- **Kiểm tra nhanh cổng + container**: từ repo gốc `takumi`:

```bash
./server-next/scripts/check-takumi-ports.sh
```

- **Server LAN trong Docker:** trong `server-next` chạy `docker compose up -d` hoặc `./scripts/docker-up.sh` — stack gồm **Postgres** và **LegacyLoginHost** (Connect **44605** + login **44606**). Cần `.env` với `TAKUMI_PUBLIC_HOST` = IP LAN Mac.

### LegacyLoginHost (.NET) vs cổng **44606**

#### Dấu hiệu vàng từ `adb logcat -s TakumiErrorReport:D`

Sau dòng **`[AndroidLogin] TCP session start … port=44606`** bạn **phải** thấy ngay (vài ms–vài chục ms) một dòng:

`[AndroidLogin] recv tcp … b0..b7=C1 …` (gói join `F1 00`, độ dài 12, byte đầu `C1`).

- **Nếu không có `recv tcp` nào trước khi bấm login** → TCP đã `connected` nhưng **server không gửi một byte**. Đây **không** phải lỗi SimpleModulus trên điện thoại; service đang listen **44606** không gửi join (hoặc container nhận login nhưng **ignore** packet — xem log Docker). Trên **LegacyLoginHost** đúng chuẩn, console có **`sent join C1 F1 00`** và logcat có **`recv tcp`** với byte đầu **`C1`**.
- **Nếu có `recv tcp` (join) nhưng sau login vẫn không có `Translate F1`** → khi đó mới xem **`Dec2.dat`** khớp host/client (`TAKUMI_DEC2_PATH`) và account (`TAKUMI_ACCOUNTS`).

Kiểm tra ai đang giữ cổng: `lsof -nP -iTCP:44606 -sTCP:LISTEN`. **Docker** (`com.docker…`) chỉ là process publish cổng — ổn nếu container thực sự là login host đúng chuẩn (join + phản hồi login). Muốn chạy **`dotnet run` trực tiếp trên Mac** trên **44606**, trước hết **`docker stop`** container đang chiếm cổng. Hoặc đổi cổng và **sửa client** cho trùng, ví dụ **44607**:

```bash
export TAKUMI_LOGIN_PORT=44607
export TAKUMI_DEC2_PATH="/absolute/path/to/takumi/.../Data/Dec2.dat"   # cùng file với Data trên điện thoại; lấy từ repo ClientBuild hoặc adb pull
cd server-next
dotnet run --project src/Takumi.Server.LegacyLoginHost/Takumi.Server.LegacyLoginHost.csproj -c Release
```

Bắt buộc thấy log **`[keys] Loaded Dec2.dat`**; nếu chỉ thấy cảnh báo “default keys” thì **login mã hóa (C3) sẽ không giải được** — `TAKUMI_DEC2_PATH` phải trỏ tới file thật, không dùng chuỗi placeholder.

### Log dừng tại `> Try to Login "…"` và **không** có `[AndroidLogin] Translate F1`

Luồng C (`LoginWin.cpp` → `SendRequestLogIn` / Android `ConnectionManager_SendLogin`) gửi login **C3 F1 01** đã mã hóa. Sau đó client chỉ xử lý tiếp khi `ProtocolCompiler` nhận được **C1 F1 01** (hoặc C3 rồi giải SM) và gọi `TranslateProtocol` — log **`[AndroidLogin] Translate F1 sub=0x01 value=0x…`** xuất hiện tại đó (`WSclient.cpp`).

Nếu **không** có `Translate F1`, thường là:

1. **Không có phản hồi TCP đúng host**: cổng game (ví dụ `44606`) đang là **Docker/nginx/OpenMU** khác, không phải `LegacyLoginHost` — kiểm tra `lsof -nP -iTCP:44606 -sTCP:LISTEN`. Process nhận kết nối phải là **`dotnet`** của host bạn chạy (hoặc đổi cổng + sửa bảng server trong client cho khớp).
2. **Giải mã login phía server thất bại**: `Dec2.dat` trên máy chạy host **khác** file trong `…/Android/data/…/files/Data/Dec2.dat` trên điện thoại → pipeline SM không ra gói plaintext → handler không trả `F1 01`.
3. **Đã có phản hồi nhưng sai mật khẩu / serial / version**: khi đó **phải** thấy `Translate F1` với `value=0x00` (sai pass) hoặc `0x06` (serial/version). Nếu log có `passLen=6` mà `TAKUMI_ACCOUNTS` vẫn mặc định `test:test` (4 ký tự), đặt lại ví dụ `TAKUMI_ACCOUNTS=test:yourSixCharPass`.

GameServer C (`CGConnectAccountRecv`) chỉ gọi `PacketArgumentDecrypt` cho **10 byte đầu** của password (`sizeof(password)-1` với `password[11]`). LegacyLoginHost đã căn theo hành vi đó.

### Nên bật

1. **`server-next`**: `cd server-next && docker compose up -d` (Postgres **54444** + LegacyLoginHost **44605**/**44606** trong Docker). Cần `.env` với `TAKUMI_PUBLIC_HOST` = IP LAN. Tuỳ chọn **`data.zip`**: `docker compose --profile datazip up -d` hoặc `./scripts/docker-up.sh --with-datazip` — URL `http://<LAN-IP>:18080/data.zip` (file `takumi/docker/data-zip/host/data.zip`). Chi tiết: `server-next/README.md`.
2. **`data.zip` chỉ qua `takumi/docker`** (khi không dùng profile trong `server-next`): từ `takumi/docker`:

   ```bash
   docker compose --profile datazip up -d
   ```

   Cùng thư mục `docker/data-zip/host/data.zip`, cổng mặc định **18080** — **tránh** bật hai stack `datazip` cùng lúc trên cùng cổng host.

### Nên tắt (khi không test OpenMU / MuServer Wine)

- Nhóm **`takumi-openmu`** (nginx + postgres OpenMU + `startup` / `munique/...`) nếu APK đã trỏ **`server-next`**.
- Profile **`db` / `wine`** của `takumi/docker/docker-compose.yml` (SQL Server + Wine MuServer) nếu bạn **không** dùng stack `.exe` trong Docker.

### Apple Silicon (AMD64 trên Docker Desktop)

Image **linux/amd64** chạy qua emulation: thường ổn cho DB + .NET, nhưng nặng hơn native. Nếu crash lạ, thu **log container** và **`adb logcat`** cùng lúc reproduce.

### App tải `data.zip` xong rồi **tự thoát** khi vào game

Đây thường là **native crash** hoặc client đóng socket sau gói không khớp server. Làm lần lượt:

1. `adb logcat` (lọc theo PID app hoặc tag `MuMain` / `SDL` / `AndroidRuntime`).
2. Log phía host `server-next` trong lúc: login → danh sách nhân vật → chọn nhân vật → bước vào map.
3. Đối chiếu `server-next/docs/IMPLEMENTATION-CHECKLIST.md` (MVP: một session login TCP vs **game port** sau select — nếu checklist ghi “chưa có”, client có thể thoát sau select).

**Tóm tối thiểu:** `server-next` (Postgres + LegacyLoginHost trong Docker) **+** tuỳ chọn **`datazip`** (profile trong `server-next` *hoặc* `takumi/docker`, cùng file zip, **một** cổng publish); tắt OpenMU/Wine/MuServer Docker khi không dùng.

---

## Chọn nhân vật trên điện thoại (touch → vào game)

Sau khi sửa native (`ZzzScene.cpp`), trên Android/iOS có thể:

- **Chạm hai lần** (double-tap) trên cùng nhân vật trong cửa sổ ngắn để gọi `StartGame()` tương tự **H.tất / Kết nối**.
- **Giữ ngón tay ~0,5 giây** trên vùng 3D (không phải thanh UI) khi đã chọn slot (`SelectedHero`) để vào game.

**Kỹ thuật:** `NewMoveCharacterScene` chạy **trước** `CInput::Update()` trong cùng frame nên không dùng `IsLBtnDn()` cho nhánh này; dùng `SEASON3B::IsPress` / `IsRepeat(VK_LBUTTON)`. Chi tiết touch + ray pick + IME sau login: **`docs/DEVELOPMENT-LOG-2026-05-12.md`**. Bổ sung **SysMenu / modal / xóa nhân vật / JNI IME Done / thứ tự `UpdateMouseFromTouch`**: **`docs/DEVELOPMENT-LOG-2026-05-14.md`**.
