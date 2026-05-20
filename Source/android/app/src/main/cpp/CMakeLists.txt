cmake_minimum_required(VERSION 3.22)
project(TakumiMuAndroid CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# android/app/src/main/cpp -> D:/takumi/Source
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
set(CLIENT_ROOT     "${PROJECT_ROOT}/5.Main")
set(GAME_SRC_DIR    "${CLIENT_ROOT}/source")
set(DEPS_ROOT       "${CLIENT_ROOT}/dependencies")
get_filename_component(JNI_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}" ABSOLUTE)

# Sokol headers — override with -DSOKOL_ROOT=... or env SOKOL_ROOT for custom checkouts.
if(DEFINED ENV{SOKOL_ROOT} AND NOT "$ENV{SOKOL_ROOT}" STREQUAL "")
    set(_sokol_default "$ENV{SOKOL_ROOT}")
else()
    set(_sokol_default "${DEPS_ROOT}/sokol")
endif()
set(SOKOL_ROOT "${_sokol_default}" CACHE PATH "Path to the local sokol checkout")
set(BGFX_ROOT "${DEPS_ROOT}/bgfx" CACHE PATH "Path to the local bgfx checkout")
set(BX_ROOT   "${DEPS_ROOT}/bx" CACHE PATH "Path to the local bx checkout")
set(BIMG_ROOT "${DEPS_ROOT}/bimg" CACHE PATH "Path to the local bimg checkout")
option(MU_ENABLE_BGFX "Build optional bgfx/bx/bimg support scaffolding" OFF)

message(STATUS "PROJECT_ROOT = ${PROJECT_ROOT}")
message(STATUS "CLIENT_ROOT  = ${CLIENT_ROOT}")
message(STATUS "GAME_SRC_DIR = ${GAME_SRC_DIR}")
message(STATUS "JNI_LIBS_DIR = ${JNI_LIBS_DIR}")
message(STATUS "SOKOL_ROOT   = ${SOKOL_ROOT}")
message(STATUS "BGFX_ROOT    = ${BGFX_ROOT}")
message(STATUS "BX_ROOT      = ${BX_ROOT}")
message(STATUS "BIMG_ROOT    = ${BIMG_ROOT}")
message(STATUS "MU_ENABLE_BGFX = ${MU_ENABLE_BGFX}")

if(NOT EXISTS "${SOKOL_ROOT}/sokol_time.h")
    message(FATAL_ERROR "Missing sokol_time.h in SOKOL_ROOT='${SOKOL_ROOT}'. Clone https://github.com/floooh/sokol into ${DEPS_ROOT}/sokol or set SOKOL_ROOT.")
endif()

add_compile_definitions(__ANDROID__ ANDROID NDEBUG _NDEBUG _NO_EDITOR USE_OPENGL_ES __NDK_FPABI__= SOKOL_GLES3)

if(MU_ENABLE_BGFX)
    if(NOT EXISTS "${BX_ROOT}/src/amalgamated.cpp")
        message(FATAL_ERROR "Missing bx amalgamated source in BX_ROOT='${BX_ROOT}'.")
    endif()
    if(NOT EXISTS "${BIMG_ROOT}/src/image.cpp")
        message(FATAL_ERROR "Missing bimg sources in BIMG_ROOT='${BIMG_ROOT}'.")
    endif()
    if(NOT EXISTS "${BGFX_ROOT}/src/amalgamated.cpp")
        message(FATAL_ERROR "Missing bgfx amalgamated source in BGFX_ROOT='${BGFX_ROOT}'.")
    endif()

    add_library(mu_bx STATIC "${BX_ROOT}/src/amalgamated.cpp")
    target_include_directories(mu_bx PUBLIC
        "${BX_ROOT}/include"
        "${BX_ROOT}/3rdparty"
    )
    target_compile_definitions(mu_bx PUBLIC
        $<$<CONFIG:Debug>:BX_CONFIG_DEBUG=1>
        $<$<NOT:$<CONFIG:Debug>>:BX_CONFIG_DEBUG=0>
    )

    file(GLOB BIMG_ASTC_SOURCES "${BIMG_ROOT}/3rdparty/astc-encoder/source/*.cpp")

    add_library(mu_bimg STATIC
        "${BIMG_ROOT}/src/image.cpp"
        "${BIMG_ROOT}/src/image_decode.cpp"
        "${BIMG_ROOT}/src/image_gnf.cpp"
        ${BIMG_ASTC_SOURCES}
    )
    target_include_directories(mu_bimg PUBLIC
        "${BIMG_ROOT}/include"
        "${BIMG_ROOT}/3rdparty"
        "${BIMG_ROOT}/3rdparty/astc-encoder/include"
        "${BIMG_ROOT}/3rdparty/tinyexr/deps"
        "${BX_ROOT}/include"
        "${BX_ROOT}/3rdparty"
    )
    target_compile_definitions(mu_bimg PUBLIC
        $<$<CONFIG:Debug>:BX_CONFIG_DEBUG=1>
        $<$<NOT:$<CONFIG:Debug>>:BX_CONFIG_DEBUG=0>
    )
    target_link_libraries(mu_bimg PUBLIC mu_bx)

    add_library(mu_bgfx STATIC "${BGFX_ROOT}/src/amalgamated.cpp")
    target_include_directories(mu_bgfx PUBLIC
        "${BGFX_ROOT}/include"
        "${BGFX_ROOT}/3rdparty"
        "${BIMG_ROOT}/include"
        "${BIMG_ROOT}/3rdparty"
        "${BX_ROOT}/include"
        "${BX_ROOT}/3rdparty"
    )
    target_compile_definitions(mu_bgfx PUBLIC
        BGFX_CONFIG_RENDERER_DIRECT3D11=0
        BGFX_CONFIG_RENDERER_DIRECT3D12=0
        BGFX_CONFIG_RENDERER_METAL=0
        BGFX_CONFIG_RENDERER_OPENGL=0
        BGFX_CONFIG_RENDERER_OPENGLES=30
        BGFX_CONFIG_RENDERER_VULKAN=0
        BGFX_CONFIG_RENDERER_WEBGPU=0
        BGFX_CONFIG_MULTITHREADED=0
        $<$<CONFIG:Debug>:BX_CONFIG_DEBUG=1>
        $<$<NOT:$<CONFIG:Debug>>:BX_CONFIG_DEBUG=0>
    )
    target_link_libraries(mu_bgfx PUBLIC mu_bimg mu_bx android log EGL GLESv3)
endif()

set(REQUIRED_PREBUILT_LIBS libSDL2.so libSDL2_image.so libSDL2_mixer.so libSDL2_ttf.so libSDL_net.so libbotan.so libmpg123.so)

foreach(REQUIRED_LIB IN LISTS REQUIRED_PREBUILT_LIBS)
    if(NOT EXISTS "${JNI_LIBS_DIR}/${REQUIRED_LIB}")
        message(FATAL_ERROR "Missing ${REQUIRED_LIB} for ABI '${ANDROID_ABI}' in ${JNI_LIBS_DIR}.")
    endif()
endforeach()

foreach(SDL_LIB SDL2 SDL2_image SDL2_mixer SDL2_ttf SDL_net)
    add_library(${SDL_LIB} SHARED IMPORTED)
    set_target_properties(${SDL_LIB} PROPERTIES IMPORTED_LOCATION "${JNI_LIBS_DIR}/lib${SDL_LIB}.so")
endforeach()

add_library(botan SHARED IMPORTED)
set_target_properties(botan PROPERTIES IMPORTED_LOCATION "${JNI_LIBS_DIR}/libbotan.so")

add_library(mpg123 SHARED IMPORTED)
set_target_properties(mpg123 PROPERTIES IMPORTED_LOCATION "${JNI_LIBS_DIR}/libmpg123.so")

file(GLOB_RECURSE GAME_SOURCES "${GAME_SRC_DIR}/*.cpp")
list(APPEND GAME_SOURCES "${GAME_SRC_DIR}/ScenePerfTelemetry.cpp")
list(APPEND GAME_SOURCES "${GAME_SRC_DIR}/ShopItemValueCache.cpp")
list(APPEND GAME_SOURCES "${GAME_SRC_DIR}/ShopPriceTrace.cpp")
list(APPEND GAME_SOURCES "${GAME_SRC_DIR}/Platform/AndroidLoginUi.cpp")
list(APPEND GAME_SOURCES "${GAME_SRC_DIR}/Platform/MobileHud.cpp")
list(APPEND GAME_SOURCES "${GAME_SRC_DIR}/Platform/MobileChatHud.cpp")
list(APPEND GAME_SOURCES "${GAME_SRC_DIR}/Platform/MobileChatUi.cpp")
list(APPEND GAME_SOURCES "${GAME_SRC_DIR}/Platform/TakumiAndroidInput.cpp")
# SVG UI raster (nanosvg). GLOB_RECURSE may skip new files until CMake reconfigure; keep explicit.
list(APPEND GAME_SOURCES "${GAME_SRC_DIR}/android_ui_svg.cpp")
# Protect.cpp uses CCRC32; glob only picks lowercase .cpp (not CCRC32.Cpp).
set(_MU_CCRC32_SRC "${GAME_SRC_DIR}/Utilities/CCRC32.Cpp")
list(APPEND GAME_SOURCES "${_MU_CCRC32_SRC}")
set_source_files_properties("${_MU_CCRC32_SRC}" PROPERTIES LANGUAGE CXX)
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*[Ww]inmain\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*MuEditor.*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*imgui.*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*MuParallel\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*GameShop.*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*ProtectSysKey.*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Scenes.*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*[/\\\\]Camera[/\\\\].*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*StackWalker.*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Stack\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*TrayMode\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*UIWindows - Copy\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Utilities/Dump.*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Utilities/Log/WindowsConsole\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Utilities/Log/ErrorReport\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Utilities/cryptopp.*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*CSResourceManager\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Utilities/Math\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*ExternalObject/Leaf.*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*DSplaysound\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*DSwaveIO\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*GCCertification\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*ExternalObject/detours\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Utilities/mapm/PI_DEMO.*\\.CPP$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Utilities/mapm/PI_DEMO.*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Utilities/mapm/.*DEMO.*\\.CPP$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*Utilities/mapm/.*DEMO.*\\.cpp$")
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*WSctlc\\.cpp$")
list(REMOVE_DUPLICATES GAME_SOURCES)

list(LENGTH GAME_SOURCES GAME_SOURCE_COUNT)
message(STATUS "Game source files count: ${GAME_SOURCE_COUNT}")

add_library(main SHARED ${GAME_SOURCES})

if(DEFINED MU_LAN_DEFAULT_HOST_A AND NOT "${MU_LAN_DEFAULT_HOST_A}" STREQUAL "")
    target_compile_definitions(main PRIVATE
        MU_LAN_DEFAULT_SERVER_HOST_A=\"${MU_LAN_DEFAULT_HOST_A}\"
        MU_LAN_DEFAULT_SERVER_HOST_W=L\"${MU_LAN_DEFAULT_HOST_A}\"
    )
endif()

if(MU_BOOTSTRAP_ADB_REVERSE STREQUAL "ON")
    target_compile_definitions(main PRIVATE MU_BOOTSTRAP_ADB_REVERSE=1)
endif()

if(DEFINED MU_GAME_TCP_PORT AND NOT "${MU_GAME_TCP_PORT}" STREQUAL "")
    target_compile_definitions(main PRIVATE MU_GAME_TCP_PORT=${MU_GAME_TCP_PORT})
endif()

set(SDL2_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL2/include")

target_include_directories(main PRIVATE
    ${SDL2_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/compat_include
    ${GAME_SRC_DIR}
    ${GAME_SRC_DIR}/ExternalObject
    ${GAME_SRC_DIR}/GameConfig
    ${GAME_SRC_DIR}/GameShop
    ${CLIENT_ROOT}/dependencies/include
    ${GAME_SRC_DIR}/OpenGL3
    ${GAME_SRC_DIR}/Platform
    ${GAME_SRC_DIR}/Scenes
    ${GAME_SRC_DIR}/Time
    ${GAME_SRC_DIR}/Translation
    ${SOKOL_ROOT}
    ${SOKOL_ROOT}/util
)

if(MU_ENABLE_BGFX)
    target_include_directories(main PRIVATE
        "${BGFX_ROOT}/include"
        "${BIMG_ROOT}/include"
        "${BX_ROOT}/include"
    )
    target_compile_definitions(main PRIVATE MU_BGFX_SUPPORT=1)
endif()

# Legacy Windows-first client on NDK/Clang: suppress high-volume, low-signal warnings.
# Concrete OOB/assignment bugs are fixed in-tree where found; remainder is legacy debt.
target_compile_options(main PRIVATE
    -Wno-unknown-pragmas
    -Wno-error=deprecated-declarations
    -Wno-error=format-security
    -Wno-format-security
    -Wno-format
    -Wno-multichar
    -Wno-narrowing
    -Wno-nonportable-include-path
    -Wno-non-c-typedef-for-linkage
    -Wno-ignored-attributes
    -Wno-macro-redefined
    -Wno-writable-strings
    -Wno-deprecated-writable-strings
    -Wno-tautological-constant-out-of-range-compare
    -Wno-tautological-pointer-compare
    -Wno-undefined-inline
    -Wno-switch
    -Wno-null-arithmetic
    -Wno-unused-value
    -Wno-missing-declarations
    -Wno-extra-tokens
    -Wno-null-conversion
    -Wno-invalid-source-encoding
    -Wno-deprecated-enum-float-conversion
    -Wno-deprecated-anon-enum-enum-conversion
    -Wno-parentheses-equality
    -Wno-parentheses
    -Wno-deprecated-enum-compare
    -Wno-implicit-const-int-float-conversion
    -Wno-literal-conversion
    -Wno-pointer-bool-conversion
    -Wno-string-compare
    -Wno-delete-abstract-non-virtual-dtor
    -Wno-delete-incomplete
    -Wno-return-type
    -Wno-pragma-once-outside-header
    -Wno-extern-initializer
    -Wno-logical-not-parentheses
    -Wno-dangling-else
    -Wno-fortify-source
    -Wno-int-to-pointer-cast
    -Wno-int-to-void-pointer-cast
    -Wno-constant-conversion
    -fno-strict-aliasing
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    $<$<CONFIG:Debug>:-O2>
    $<$<CONFIG:Debug>:-g1>
)

target_link_options(main PRIVATE
    -Wl,-z,max-page-size=16384
)

target_link_libraries(main PRIVATE SDL2 SDL2_image SDL2_mixer SDL2_ttf SDL_net mpg123 botan android log GLESv3 EGL z c++_static)

if(MU_ENABLE_BGFX)
    target_link_libraries(main PRIVATE mu_bgfx)
endif()















