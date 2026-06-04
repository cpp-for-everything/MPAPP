# SPDX-License-Identifier: Apache-2.0
# T-0011 — Android example native CMakeLists. Invoked by Gradle's
# externalNativeBuild with `ANDROID_PLATFORM`, `ANDROID_ABI`, etc.
# pre-set.
#
# Per ADR-0013 the handler source list is a glob — adding a new
# widget's *_handler.cpp doesn't require touching this file.

cmake_minimum_required(VERSION 3.28)

project(android_hello CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MPAPP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../..")

file(GLOB MPAPP_ANDROID_HANDLERS CONFIGURE_DEPENDS
    ${MPAPP_ROOT}/src/handlers/android/*.cpp
)

# Real per-platform Essentials backends (clipboard, vibration, ...) — JNI
# implementations behind the neutral interfaces. Globbed like the handlers.
file(GLOB MPAPP_ANDROID_ESSENTIALS CONFIGURE_DEPENDS
    ${MPAPP_ROOT}/src/essentials/android/*.cpp
)

# ADR-0015 graphics facade — backend-independent value types are
# always compiled; the backend .cpp is selected based on
# MPAPP_GRAPHICS_BACKEND. The graphics_sources list is appended to
# the android_hello library below.
set(MPAPP_GRAPHICS_SOURCES
    ${MPAPP_ROOT}/src/detail/graphics/canvas.cpp
    ${MPAPP_ROOT}/src/detail/graphics/shape_renderer.cpp
)

# MPAPP_GRAPHICS_BACKEND is set by Gradle's externalNativeBuild
# arguments (see app/build.gradle.kts). Default to cairo so Android
# follows the same convention as the host build.
set(MPAPP_GRAPHICS_BACKEND "cairo" CACHE STRING
    "2D graphics backend: cairo (default), stub")

set(MPAPP_GRAPHICS_HAS_CAIRO OFF)
set(MPAPP_GRAPHICS_HAS_SKIA  OFF)
if(MPAPP_GRAPHICS_BACKEND STREQUAL "cairo")
    # Gradle passes MPAPP_CAIRO_PREFIX pointing at vcpkg's
    # installed/<triplet>-android directory. Bridge it into the
    # PKG_CONFIG_PATH env var that pkg_check_modules reads at
    # invocation time.
    if(DEFINED MPAPP_CAIRO_PREFIX)
        set(ENV{PKG_CONFIG_PATH} "${MPAPP_CAIRO_PREFIX}/lib/pkgconfig")
    endif()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(MPAPP_CAIRO QUIET cairo)
    endif()
    if(MPAPP_CAIRO_FOUND)
        list(APPEND MPAPP_GRAPHICS_SOURCES
            ${MPAPP_ROOT}/src/detail/graphics/cairo_backend.cpp)
        set(MPAPP_GRAPHICS_HAS_CAIRO ON)
        message(STATUS
            "android_hello graphics backend: cairo ${MPAPP_CAIRO_VERSION}")
    else()
        list(APPEND MPAPP_GRAPHICS_SOURCES
            ${MPAPP_ROOT}/src/detail/graphics/stub_backend.cpp)
        message(STATUS
            "android_hello graphics backend: stub (cairo not found via pkg-config)")
    endif()
elseif(MPAPP_GRAPHICS_BACKEND STREQUAL "skia")
    # Default flow: `cmake/MpappFindSkia.cmake` auto-downloads the pinned
    # HumbleUI/SkiaBuild Android prebuilt for the current ABI
    # (x86_64 → x64, arm64-v8a → arm64) via FetchContent and assembles
    # the `unofficial::skia::skia` imported target from it. No vcpkg
    # required — sidesteps the vcpkg Windows-host cross-compile bugs
    # entirely (see vault/50_Tasks/T-0030-skia-backend/notes/dual-vcpkg-roots.md).
    #
    # Override path: pass `-PmpappSkiaPrefix=<dir>` to Gradle (or
    # `-DMPAPP_SKIA_PREFIX=<dir>` directly to CMake) to point at a
    # vcpkg installed/<triplet>-android dir or a manually unzipped
    # HumbleUI / JetBrains drop. The helper detects either layout.
    list(APPEND CMAKE_MODULE_PATH "${MPAPP_ROOT}/cmake")
    include(MpappFindSkia)
    mpapp_find_skia()
    if(MPAPP_SKIA_FOUND)
        list(APPEND MPAPP_GRAPHICS_SOURCES
            ${MPAPP_ROOT}/src/detail/graphics/skia_backend.cpp)
        set(MPAPP_GRAPHICS_HAS_SKIA ON)
        message(STATUS
            "android_hello graphics backend: skia (${MPAPP_SKIA_LAYOUT})")
    else()
        list(APPEND MPAPP_GRAPHICS_SOURCES
            ${MPAPP_ROOT}/src/detail/graphics/stub_backend.cpp)
        message(STATUS
            "android_hello graphics backend: stub (no prebuilt for "
            "ABI=${CMAKE_ANDROID_ARCH_ABI} and MPAPP_SKIA_PREFIX unset)")
    endif()
else()
    list(APPEND MPAPP_GRAPHICS_SOURCES
        ${MPAPP_ROOT}/src/detail/graphics/stub_backend.cpp)
    message(STATUS "android_hello graphics backend: stub")
endif()

add_library(android_hello SHARED
    native_main.cpp
    ${MPAPP_ROOT}/src/mpapp.cpp
    ${MPAPP_ROOT}/src/executor/mock.cpp
    ${MPAPP_GRAPHICS_SOURCES}
    ${MPAPP_ANDROID_HANDLERS}
    ${MPAPP_ANDROID_ESSENTIALS}
)

target_include_directories(android_hello PRIVATE "${MPAPP_ROOT}/include")
# T-0029 phase 2: graphics_view real handler uses AndroidBitmap_*
# from <android/bitmap.h> to lock + memcpy pixels into a managed
# Bitmap. That API ships as a separate stub library (jnigraphics) in
# the NDK.
target_link_libraries(android_hello PRIVATE log android jnigraphics)

if(MPAPP_GRAPHICS_HAS_CAIRO)
    target_include_directories(android_hello PRIVATE
        ${MPAPP_CAIRO_INCLUDE_DIRS})
    target_link_libraries(android_hello PRIVATE
        ${MPAPP_CAIRO_LIBRARIES}
        # Android bionic exposes iconv via API 28+. fontconfig pulls
        # iconv_open / iconv / iconv_close in its FcSfntNameTranscode
        # path. iconv ships in libc on API 28+ so no separate -liconv
        # link is needed (matches the Android NDK contract).
    )
    target_link_directories(android_hello PRIVATE
        ${MPAPP_CAIRO_LIBRARY_DIRS})
    target_compile_definitions(android_hello PRIVATE MPAPP_GRAPHICS_HAS_CAIRO=1)
endif()

if(MPAPP_GRAPHICS_HAS_SKIA)
    # vcpkg's unofficial-skia config exports a single imported target
    # that carries Skia + its transitive deps (libpng, libjpeg,
    # plus the minimal feature set installed for android — typically
    # no harfbuzz / ICU since text shaping isn't needed for ShapeView
    # / GraphicsView rendering).
    target_link_libraries(android_hello PRIVATE unofficial::skia::skia)
    target_compile_definitions(android_hello PRIVATE MPAPP_GRAPHICS_HAS_SKIA=1)
endif()
