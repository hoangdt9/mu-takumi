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
./gradlew :app:assembleRealDeviceDebug \
  -PmuRequiredAbis=armeabi-v7a,arm64-v8a \
  -PmuFailOnMissingRequiredAbis=true
```

Output: `app/build/outputs/apk/realDevice/debug/app-realDevice-debug.apk` (exact path may vary slightly by AGP).

## Build APK (emulator — x86)

```bash
./gradlew :app:assembleEmulatorDebug \
  -PmuRequiredAbis=x86,x86_64 \
  -PmuFailOnMissingRequiredAbis=true
```

## Install on device / emulator

```bash
adb install -r app/build/outputs/apk/realDevice/debug/*.apk
```

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

**Tóm:** Trên Mac chỉ **build Android**; server test = **Windows (máy hoặc VM)** + **mạng** + **IP client đúng**.
