Toolchain

  - Android Studio có JDK 17 vì project đang trỏ tới JBR của Android Studio trong /D:/takumi/New folder (2)/Source/android/gradle.properties:2
  - Android SDK ở /D:/takumi/New folder (2)/Source/android/local.properties:1: C:/Users/thanh/AppData/Local/Android/Sdk
  - Android SDK Platform 34 vì /D:/takumi/New folder (2)/Source/android/app/build.gradle:273 dùng compileSdk 34
  - Android NDK đang dùng bản 25.1.8937393 trong máy build hiện tại
  - CMake 3.22.1 vì /D:/takumi/New folder (2)/Source/android/app/build.gradle:323 chỉ định version "3.22.1"
  - Gradle 8.7 qua wrapper trong /D:/takumi/New folder (2)/Source/android/gradle/wrapper/gradle-wrapper.properties:3
  - Android Gradle Plugin 8.2.2 trong /D:/takumi/New folder (2)/Source/android/build.gradle:3

  Thư viện Java/Android

  - androidx.appcompat:appcompat:1.6.1 trong /D:/takumi/New folder (2)/Source/android/app/build.gradle:369

  Thư viện native .so bắt buộc
  Theo /D:/takumi/New folder (2)/Source/android/app/build.gradle:8, mỗi ABI cần đủ các file này trong android/app/src/main/jniLibs/<abi>/:

  - libSDL2.so
  - libSDL2_image.so
  - libSDL2_mixer.so
  - libSDL2_ttf.so
  - libSDL_net.so
  - libbotan.so
  - libmpg123.so

  ABI hiện có
  Thư mục /D:/takumi/New folder (2)/Source/android/app/src/main/jniLibs đang có đủ cho:

  - armeabi-v7a
  - arm64-v8a
  - x86
  - x86_64

  Nói ngắn gọn: nếu máy đã có Android Studio + SDK 34 + NDK 25.1.8937393 + CMake 3.22.1, và thư mục jniLibs không bị thiếu các .so trên, thì build được.


powershell
  Set-Location 'D:\takumi\New folder (2)\Source\android'
  & .\gradlew.bat clean
  & .\gradlew.bat ':app:assembleRealDeviceRelease' '-PmuRequiredAbis=armeabi-v7a,arm64-v8a' '-PmuFailOnMissingRequiredAbis=true'

  Các lệnh build hay dùng:

  & .\gradlew.bat ':app:assembleRealDeviceDebug' '-PmuRequiredAbis=armeabi-v7a,arm64-v8a' '-PmuFailOnMissingRequiredAbis=true'
  & .\gradlew.bat ':app:assembleRealDeviceRelease' '-PmuRequiredAbis=armeabi-v7a,arm64-v8a' '-PmuFailOnMissingRequiredAbis=true'
  & .\gradlew.bat ':app:assembleEmulatorDebug' '-PmuRequiredAbis=x86,x86_64' '-PmuFailOnMissingRequiredAbis=true'
  & .\gradlew.bat ':app:assembleEmulatorRelease' '-PmuRequiredAbis=x86,x86_64' '-PmuFailOnMissingRequiredAbis=true'
  & .\gradlew.bat ':app:assembleUniversalDebug' '-PmuRequiredAbis=armeabi-v7a,arm64-v8a,x86,x86_64' '-PmuFailOnMissingRequiredAbis=true'
  & .\gradlew.bat ':app:assembleUniversalRelease' '-PmuRequiredAbis=armeabi-v7a,arm64-v8a,x86,x86_64' '-PmuFailOnMissingRequiredAbis=true'

### Fix IP và URL `data.zip` (bản Takumi hiện tại)

- **Server IP / cổng (native):** các file trong `Source/5.Main/source/` (ví dụ `android_main.cpp`, `GameConfig/GameConfigConstants.h`, `LauncherHelper.h`, `Winmain.cpp`, `Scenes/SceneCore.cpp`) — chỉnh theo IP máy chạy Connect/Login thật.
- **`data.zip` (Android):** không sửa cứng một dòng trong `PreloadActivity` nữa; thứ tự URL do `buildDataZipUrlCandidates()`:
  1. `BuildConfig.DATA_ZIP_URL_LAN` (Gradle `DATA_ZIP_URL_LAN`, mặc định trong `app/build.gradle`; override `-PmuDataZipLan=http://<LAN>:18080/data.zip`).
  2. Fallback `http://update.daybreak.id.vn/update/data.zip`.

Chi tiết build Mac: `docs/ANDROID-DEV-MAC.md`.

**Nhật ký thay đổi client (chọn nhân vật / touch):** `docs/DEVELOPMENT-LOG-2026-05-12.md`.


cd /Users/hoangmac/Project/MU/takumi/Source/android
chmod +x ./gradlew
./gradlew :app:assembleRealDevicePreloadDefaultDebug \
  -PmuRequiredAbis=armeabi-v7a,arm64-v8a \
  -PmuFailOnMissingRequiredAbis=true

cd /Users/hoangmac/Project/MU/takumi/Source/android
adb install -r app/build/outputs/apk/realDevicePreloadDefault/debug/*.apk 