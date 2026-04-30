@echo off
:: copy_libs.bat
:: Copies prebuilt .so files from APK reference into jniLibs/<ABI>/.
::
:: Usage:
::   copy_libs.bat
::   copy_libs.bat "D:\path\to\extracted\lib"
::
:: Default source:
::   ..\tool\ARGMU_v2728\apk_extracted\lib

setlocal EnableExtensions

set "APK_LIB_ROOT=%~1"
if "%APK_LIB_ROOT%"=="" set "APK_LIB_ROOT=..\tool\ARGMU_v2728\apk_extracted\lib"
set "DEST_ROOT=app\src\main\jniLibs"
set "ARM64_BOTAN_SAFE=thirdparty_build\tmp\libbotan_relinked16k_norelro.so"
set "HAS_MISSING=0"

call :copy_abi "armeabi-v7a"
call :copy_abi "arm64-v8a"

echo.
if "%HAS_MISSING%"=="1" (
    echo [WARN] One or more required files were missing.
    echo [WARN] Build still works for ABIs that were copied completely.
) else (
    echo Done. Native libraries copied successfully.
)
echo Source root: %APK_LIB_ROOT%
echo Destination root: %DEST_ROOT%
echo.
echo Strict ABI check:
echo   .\gradlew.bat :app:assembleDebug -PmuRequiredAbis=armeabi-v7a,arm64-v8a -PmuFailOnMissingRequiredAbis=true
pause
exit /b 0

:copy_abi
set "ABI=%~1"
set "SRC=%APK_LIB_ROOT%\%ABI%"
set "DEST=%DEST_ROOT%\%ABI%"

echo.
echo ==== Copy ABI: %ABI% ====
if not exist "%SRC%" (
    echo [WARN] Source folder not found: %SRC%
    set "HAS_MISSING=1"
    exit /b 0
)

if not exist "%DEST%" mkdir "%DEST%"

call :copy_required "%SRC%\libSDL2.so" "%DEST%\libSDL2.so"
call :copy_required "%SRC%\libSDL2_image.so" "%DEST%\libSDL2_image.so"
call :copy_with_fallback "%SRC%\libSDL2_mixer.so" "%SRC%\libSDL2_Mix.so" "%DEST%\libSDL2_mixer.so"
call :copy_required "%SRC%\libSDL2_ttf.so" "%DEST%\libSDL2_ttf.so"
call :copy_with_fallback "%SRC%\libSDL_net.so" "%SRC%\libSDL2_net.so" "%DEST%\libSDL_net.so"
if /I "%ABI%"=="arm64-v8a" (
    if exist "%ARM64_BOTAN_SAFE%" (
        copy /Y "%ARM64_BOTAN_SAFE%" "%DEST%\libbotan.so" >nul
        echo [INFO] Using safe 16KB arm64 libbotan.so from %ARM64_BOTAN_SAFE%
    ) else if exist "%DEST%\libbotan.so" (
        echo [WARN] Safe arm64 botan stub not found at %ARM64_BOTAN_SAFE%
        echo [WARN] Keeping existing %DEST%\libbotan.so
    ) else (
        echo [WARN] Missing safe arm64 botan stub: %ARM64_BOTAN_SAFE%
        set "HAS_MISSING=1"
    )
) else (
    call :copy_required "%SRC%\libbotan.so" "%DEST%\libbotan.so"
)
call :copy_required "%SRC%\libmpg123.so" "%DEST%\libmpg123.so"

if exist "%SRC%\libhidapi.so" (
    copy /Y "%SRC%\libhidapi.so" "%DEST%\libhidapi.so" >nul
)

echo [OK] Finished ABI: %ABI%
exit /b 0

:copy_required
if exist "%~1" (
    copy /Y "%~1" "%~2" >nul
) else (
    echo [WARN] Missing required file: %~1
    set "HAS_MISSING=1"
)
exit /b 0

:copy_with_fallback
if exist "%~1" (
    copy /Y "%~1" "%~3" >nul
    exit /b 0
)

if exist "%~2" (
    copy /Y "%~2" "%~3" >nul
    exit /b 0
)

echo [WARN] Missing required file: %~1 or %~2
set "HAS_MISSING=1"
exit /b 0
