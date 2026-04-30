# Takumi Android -> Sokol

Nguon `sokol` dang dung:

- `D:/firstproject/sokol-master`

Tinh trang du an Android hien tai:

- Bootstrap app dang di qua `SDLActivity` -> `SDL_main()` trong `5.Main/source/android_main.cpp`.
- Render dang dung `Platform/gl_compat.cpp`: lop emulation OpenGL fixed-function tren GLES3.
- SDL hien con dinh vao 4 cum chinh:
  - app lifecycle + event loop: `android_main.cpp`
  - timing/perf counters: nhieu file hot-path
  - soft keyboard + keyboard state: `PlatformDefs.h`, `UIControls.cpp`, `CharMakeWin.cpp`
  - font/audio/filesystem Android helper: `AndroidGDI.cpp`, `SDL_mixer`, `SDL_AndroidGet*`

Buoc da lam trong dot nay:

- Timing/perf da duoc tach khoi SDL sang `sokol_time.h`.
- Android CMake da nhan `SOKOL_ROOT` va include truc tiep local checkout `D:/firstproject/sokol-master`.
- Wrapper moi:
  - `Platform/MobileTime.h`
  - `Platform/MobileTime.cpp`

Y nghia:

- Toan bo `GetTickCount`, `QueryPerformanceCounter`, `QueryPerformanceFrequency` cua Android port khong con phu thuoc vao SDL timer nua.
- Day la buoc nen de sau do thay `SDL_main`/`SDLActivity` bang `sokol_app.h` ma khong phai sua lai telemetry va frame timing lan nua.

Buoc tiep theo de chuyen sang sokol day du:

1. Tao `sokol_app` bootstrap Android thay cho `SDLActivity`/`SDL_main`.
2. Chuyen input/touch/text-input sang `sapp_event`.
3. Giu `gl_compat` tam thoi, nhung doi lifecycle/context sang `sokol_app`.
4. Sau khi bootstrap on dinh, thay dan `gl_compat` bang `sokol_gl.h` + `sokol_gfx.h`.
5. Cuoi cung moi loai bo SDL khoi font/audio hoac giu lai tung phan neu can.
