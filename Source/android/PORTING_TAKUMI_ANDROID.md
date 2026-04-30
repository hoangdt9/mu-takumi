# Takumi Android Port Scaffold

This `android` folder is structured for the official project at `D:\takumi\Source`.

## Source Roots

- Project root: `D:\takumi\Source`
- Client root: `D:\takumi\Source\5.Main`
- Game source root: `D:\takumi\Source\5.Main\source`
- Android app root: `D:\takumi\Source\android\app`

## ABI Coverage

`android/app/src/main/jniLibs` contains native libraries for:

- `armeabi-v7a` real 32-bit devices
- `arm64-v8a` real 64-bit devices
- `x86` 32-bit emulator
- `x86_64` 64-bit emulator

Gradle flavors:

- `realDeviceDebug` / `realDeviceRelease`: `armeabi-v7a`, `arm64-v8a`
- `emulatorDebug` / `emulatorRelease`: `x86`, `x86_64`
- `universalDebug` / `universalRelease`: all detected complete ABI sets

## Data Update

`PreloadActivity` downloads and extracts:

- `http://update.daybreak.id.vn/update/data.zip`

The URL is already present in:

- `android/app/src/main/java/com/muonline/client/PreloadActivity.java`

## Current Port State

The Android Gradle/CMake scaffold is in place and retargeted to `D:\takumi\Source\5.Main`.
The project also has Android support code copied into `5.Main/source` from the completed reference port:

- `android`
- `Platform`
- `GameConfig`
- `Scenes`
- `Translation`
- `android_main.cpp`

This is a real porting base, not a final compile guarantee. The Takumi client is not identical to the reference project, so the next step is to build and fix compile errors in the platform/render/audio/network merge layer.

## Build Commands

From `D:\takumi\Source\android`:

```bat
gradlew.bat :app:assembleRealDeviceDebug -PmuRequiredAbis=armeabi-v7a,arm64-v8a -PmuFailOnMissingRequiredAbis=true
```

```bat
gradlew.bat :app:assembleEmulatorDebug -PmuRequiredAbis=x86,x86_64 -PmuFailOnMissingRequiredAbis=true
```

```bat
gradlew.bat :app:assembleUniversalDebug -PmuRequiredAbis=armeabi-v7a,arm64-v8a,x86,x86_64 -PmuFailOnMissingRequiredAbis=true
```
