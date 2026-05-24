cmake_minimum_required(VERSION 3.28)

project(mpapp
    VERSION 0.0.1
    DESCRIPTION "MPAPP - C++ cross-platform UI framework"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake 3.28 enables clang-scan-deps for C++23 by default. MPAPP doesn't
# use named modules in its public surface yet, and several toolchains
# (Ubuntu clang 18, Zig 0.13) ship clang/clang++ without a matching
# clang-scan-deps. Disable globally so the build does not depend on
# scan-deps being on PATH.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# MSVC 14.51+ emits C4530 from <chrono>/<format>/<filesystem> headers unless
# unwind semantics are enabled. Visual Studio's MSBuild generator implicitly
# adds /EHsc; the Ninja generator does not. Set it project-wide so /WX builds
# stay green.
if(MSVC)
    add_compile_options(/EHsc)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Host-platform tools (mpapp CLI, mpapp-xc, ...) are on by default for native
# builds, off when cross-compiling. CMAKE_CROSSCOMPILING is set by toolchain
# files; when truthy we default off so a cross-target build does not also pull
# in the host CLI. Overridable from the command line.
if(CMAKE_CROSSCOMPILING)
    set(MPAPP_BUILD_TOOLS_DEFAULT OFF)
else()
    set(MPAPP_BUILD_TOOLS_DEFAULT ON)
endif()
option(MPAPP_BUILD_TOOLS
    "Build host-platform tools (mpapp CLI, mpapp-xc XAML compiler, ...)."
    ${MPAPP_BUILD_TOOLS_DEFAULT}
)

# Example programs (platform spikes). Default-on where their platform prereqs
# are routinely available — Windows (WinUI 3) and Linux/Unix (GTK4 via apt /
# pkg-config). macOS/iOS/Android spikes get the same treatment as their build
# prereqs land.
if(WIN32 OR (UNIX AND NOT APPLE))
    option(MPAPP_BUILD_EXAMPLES "Build the platform example apps." ON)
else()
    option(MPAPP_BUILD_EXAMPLES "Build the platform example apps." OFF)
endif()

# mpapp-core static library. Sources accumulate by platform. The platform
# UI handlers (WinUI 3 button/label, GTK4, AppKit, UIKit) are NOT compiled
# into mpapp-core because they pull in heavy native SDKs (WindowsAppSDK,
# C++/WinRT projections, ...). Each example/app builds its own
# mpapp-handlers-<platform> static library that compiles those .cpp files
# with the SDK include paths set up. mpapp-core stays platform-neutral.
set(MPAPP_CORE_SOURCES src/mpapp.cpp)

# Async executor — mock (deterministic) backing for mpapp::main_dispatcher().
# Cross-platform; real per-platform sources (windows_iocp.cpp, linux_iouring.cpp,
# ...) are stubbed and land in P3+. See vault/10_Architecture/Async Executor and
# Event Loops.md.
list(APPEND MPAPP_CORE_SOURCES src/executor/mock.cpp)

# 2D graphics facade per ADR-0015. Backend-independent sources
# (canvas value types, SVG path parser) compile always. One backend
# .cpp is selected via the MPAPP_GRAPHICS_BACKEND option.
#
#   cairo  — LGPL via dynamic linking (per RFC-0001). Default where
#            available — Linux via GTK4's libcairo + pkg-config,
#            Windows via vcpkg cairo (pkg-config bridge through
#            vcpkg's bundled mingw64 pkgconf), Android via vcpkg
#            arm64-android / x64-android triplets (wired through
#            Gradle's externalNativeBuild — see
#            examples/android_hello/app/build.gradle.kts). On
#            platforms where Cairo isn't detected, falls back to
#            stub with a CMake warning so the build stays green.
#   stub   — records calls without rendering. Useful for tests +
#            headless builds; no native dependency.
#   skia   — BSD-3, ~30 MB add. Implementation still TBD.
#
# Default is `cairo` on every platform — the detection logic below
# falls back to stub when Cairo isn't actually installed, so the
# practical behavior matches what each developer has set up.
set(MPAPP_GRAPHICS_BACKEND "cairo" CACHE STRING
    "2D graphics backend: cairo (default; auto-falls-back to stub if not found), stub, or skia (opt-in via vcpkg)")
set_property(CACHE MPAPP_GRAPHICS_BACKEND PROPERTY STRINGS stub cairo skia)

list(APPEND MPAPP_CORE_SOURCES
    src/detail/graphics/canvas.cpp
    src/detail/graphics/shape_renderer.cpp)

set(MPAPP_GRAPHICS_HAS_CAIRO OFF)
set(MPAPP_GRAPHICS_HAS_SKIA  OFF)
if(MPAPP_GRAPHICS_BACKEND STREQUAL "cairo")
    # pkg-config is the detection path on all three platforms. vcpkg
    # ships cairo with .pc files and bundles mingw64 pkgconf in
    # downloads/tools/msys2; the build script (_build_full.bat) wires
    # `PKG_CONFIG_EXECUTABLE` to that pkgconf so this same find logic
    # works regardless of host.
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(MPAPP_CAIRO QUIET cairo)
    endif()
    if(MPAPP_CAIRO_FOUND)
        list(APPEND MPAPP_CORE_SOURCES src/detail/graphics/cairo_backend.cpp)
        set(MPAPP_GRAPHICS_HAS_CAIRO ON)
        message(STATUS "MPAPP graphics backend: cairo (${MPAPP_CAIRO_VERSION})")
    else()
        list(APPEND MPAPP_CORE_SOURCES src/detail/graphics/stub_backend.cpp)
        message(WARNING
            "MPAPP_GRAPHICS_BACKEND=cairo selected but libcairo not found via "
            "pkg-config. Falling back to stub backend.\n"
            "  Linux:   apt install libcairo2-dev pkg-config\n"
            "  Windows: vcpkg install cairo:x64-windows, then pass\n"
            "             -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake\n"
            "             -DPKG_CONFIG_EXECUTABLE=<vcpkg>/downloads/tools/msys2/.../mingw64/bin/pkgconf.exe\n"
            "  Android: vcpkg install cairo:x64-android (or arm64-android), then see\n"
            "             examples/android_hello/app/build.gradle.kts for the Gradle wiring.")
    endif()
elseif(MPAPP_GRAPHICS_BACKEND STREQUAL "stub")
    list(APPEND MPAPP_CORE_SOURCES src/detail/graphics/stub_backend.cpp)
    message(STATUS "MPAPP graphics backend: stub")
elseif(MPAPP_GRAPHICS_BACKEND STREQUAL "skia")
    # Skia is heavyweight (BSD-3, ~30 MB linked) and opt-in. Two install
    # layouts are supported — see cmake/MpappFindSkia.cmake for the
    # detection rules:
    #
    #   1. vcpkg's `unofficial-skia` CMake config package (the canonical
    #      Windows/Linux path; builds Skia from source on first install).
    #   2. Community prebuilt drops from HumbleUI/SkiaBuild or
    #      JetBrains/skia-pack (workaround for Android-from-Windows-host,
    #      where vcpkg's GN build hits upstream tooling bugs — see
    #      vault/50_Tasks/T-0030-skia-backend/notes/dual-vcpkg-roots.md).
    #
    # Either way the helper exposes a uniform `unofficial::skia::skia`
    # imported target. If neither layout produces one, fall back to stub
    # so the rest of the build keeps compiling — same graceful-fallback
    # pattern Cairo uses.
    include(MpappFindSkia)
    mpapp_find_skia()
    if(MPAPP_SKIA_FOUND)
        list(APPEND MPAPP_CORE_SOURCES src/detail/graphics/skia_backend.cpp)
        set(MPAPP_GRAPHICS_HAS_SKIA ON)
        message(STATUS "MPAPP graphics backend: skia (${MPAPP_SKIA_LAYOUT})")
    else()
        list(APPEND MPAPP_CORE_SOURCES src/detail/graphics/stub_backend.cpp)
        message(WARNING
            "MPAPP_GRAPHICS_BACKEND=skia selected but Skia not found. "
            "Falling back to stub backend.\n"
            "  Install (vcpkg, builds from source ~30 min first time):\n"
            "    Windows:  vcpkg install skia:x64-windows\n"
            "    Linux:    vcpkg install skia:x64-linux\n"
            "    Android:  vcpkg install skia:arm64-android (or x64-android)\n"
            "              — Windows-host → Android cross-compile hits two\n"
            "              upstream tooling bugs; use a Linux NDK or the\n"
            "              prebuilt path below.\n"
            "  Then pass -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake.\n"
            "  Or point MPAPP_SKIA_PREFIX at a HumbleUI/SkiaBuild or\n"
            "  JetBrains/skia-pack prebuilt drop (https://github.com/HumbleUI/SkiaBuild\n"
            "  / https://github.com/JetBrains/skia-pack).")
    endif()
else()
    message(FATAL_ERROR
        "Unknown MPAPP_GRAPHICS_BACKEND='${MPAPP_GRAPHICS_BACKEND}'. "
        "Expected stub / cairo / skia.")
endif()

if(WIN32)
    list(APPEND MPAPP_CORE_SOURCES src/hot_reload/windows.cpp)
endif()

add_library(mpapp-core STATIC ${MPAPP_CORE_SOURCES})

target_include_directories(mpapp-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Cairo backend wiring — include paths + link flags from pkg-config.
# Include path is PUBLIC because tests in this repo include
# `<cairo/cairo.h>` directly for backend-specific pixel-readback
# verification. End-user code outside this repo should only use the
# abstract `mpapp::detail::graphics::canvas` interface and stay
# backend-agnostic.
if(MPAPP_GRAPHICS_HAS_CAIRO)
    target_include_directories(mpapp-core PUBLIC ${MPAPP_CAIRO_INCLUDE_DIRS})
    target_link_libraries(mpapp-core PUBLIC ${MPAPP_CAIRO_LIBRARIES})
    target_link_directories(mpapp-core PUBLIC ${MPAPP_CAIRO_LIBRARY_DIRS})
    target_compile_definitions(mpapp-core PUBLIC MPAPP_GRAPHICS_HAS_CAIRO=1)
endif()

# Skia backend wiring — the vcpkg CMake config exposes a single
# imported target `unofficial::skia::skia` that carries include
# directories, lib paths, and the (long) list of transitive deps
# (libpng, freetype, harfbuzz, expat, ...). Linking just the target
# is enough; vcpkg pulls everything else through.
if(MPAPP_GRAPHICS_HAS_SKIA)
    target_link_libraries(mpapp-core PUBLIC unofficial::skia::skia)
    target_compile_definitions(mpapp-core PUBLIC MPAPP_GRAPHICS_HAS_SKIA=1)
endif()

target_compile_features(mpapp-core PUBLIC cxx_std_23)

target_compile_options(mpapp-core PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
)

# Testing -- initialized before any subdirectory so nested CMakeLists can
# observe BUILD_TESTING and register their own targets via catch_discover_tests.
include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Developer tools (mpapp CLI, mpapp-xc XAML compiler, ...). Gated on
# MPAPP_BUILD_TOOLS so cross-compiles skip cleanly. Added after tests so the
# tool test binaries can reuse the Catch2::Catch2WithMain target the tests
# subdirectory fetched.
if(MPAPP_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

# Platform-specific example apps (WinUI 3 button spike, hot-reload spike, ...).
if(MPAPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
