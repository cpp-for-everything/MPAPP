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
    # Gradle passes MPAPP_SKIA_PREFIX pointing at one of two layouts:
    #
    #   1. vcpkg's installed/<triplet>-android directory (the canonical
    #      path; needs `vcpkg install skia[core,png,jpeg]:<triplet>`).
    #   2. A HumbleUI/SkiaBuild or JetBrains/skia-pack prebuilt drop
    #      (unzipped into <prefix>/, contains `out/Release-<arch>/libskia.a`
    #      plus headers under `include/` — see
    #      cmake/MpappFindSkia.cmake for the layout detection).
    #
    # The prebuilt path exists specifically because vcpkg's GN build
    # currently fails when cross-compiling Skia → Android from a
    # Windows host (two stacked upstream tooling bugs, see
    # vault/50_Tasks/T-0030-skia-backend/notes/dual-vcpkg-roots.md).
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
            "android_hello graphics backend: stub (skia not found at MPAPP_SKIA_PREFIX)")
    endif()
else()
    list(APPEND MPAPP_GRAPHICS_SOURCES
        ${MPAPP_ROOT}/src/detail/graphics/stub_backend.cpp)
    message(STATUS "android_hello graphics backend: stub")
endif()

add_library(android_hello SHARED
    native_main.cpp
    ${MPAPP_ROOT}/src/mpapp.cpp
    ${MPAPP_GRAPHICS_SOURCES}
    ${MPAPP_ANDROID_HANDLERS}
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
