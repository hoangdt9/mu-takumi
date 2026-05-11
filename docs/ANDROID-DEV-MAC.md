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

### Nên bật

1. **`server-next`**: **Postgres** trong repo — `cd server-next && docker compose up -d` (container `takumi-next-postgres`, cổng host mặc định **54444**). **Host .NET** chạy trên máy (hoặc image riêng) khi đã có lại mã nguồn trong `server-next/src`; publish cổng khớp `TAKUMI_CONNECT_PORT` / `TAKUMI_LOGIN_PORT` và IP `TAKUMI_PUBLIC_HOST` mà **điện thoại cùng Wi‑Fi** truy cập được (xem `server-next/README.md`).
2. **`data.zip` qua HTTP** (chỉ khi cần tải/ghi đè bundle): từ `takumi/docker`:

   ```bash
   docker compose --profile datazip up -d
   ```

   Đặt `docker/data-zip/host/data.zip`, cổng mặc định **18080** (xem `docker/README.md`).

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

**Tóm tối thiểu:** `server-next` (Postgres + host) **+** tuỳ chọn **`datazip`**; tắt OpenMU/Wine/MuServer Docker khi không dùng.
