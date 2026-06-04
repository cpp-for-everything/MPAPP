# cmake/MpappOptions.cmake
#
# Project-wide build options and global C++ settings for a CHILD build (one
# platform). Included once from the top-level CMakeLists.txt in child mode,
# immediately after project(). The superbuild orchestrator does NOT include
# this — it owns no real targets.
#
# Everything here used to live inline at the top of the root CMakeLists.txt;
# it was moved out so the root stays a thin mode-dispatcher.
include_guard(GLOBAL)

# -- Language baseline ---------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake 3.28 enables clang-scan-deps for C++23 by default. MPAPP doesn't use
# named modules in its public surface yet, and several toolchains (Ubuntu
# clang 18, Zig 0.13) ship clang/clang++ without a matching clang-scan-deps.
# Disable globally so the build does not depend on scan-deps being on PATH.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Emit compile_commands.json — feeds clang-tidy, clangd, and other tooling.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# MSVC 14.51+ emits C4530 from <chrono>/<format>/<filesystem> headers unless
# unwind semantics are enabled. Visual Studio's MSBuild generator implicitly
# adds /EHsc; the Ninja generator does not. Set it project-wide so /WX builds
# stay green.
if(MSVC)
    add_compile_options(/EHsc)
endif()

# -- Options -------------------------------------------------------------------
#
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
# prereqs land. Cross-compiled children skip examples (they need native SDKs).
if((WIN32 OR (UNIX AND NOT APPLE)) AND NOT CMAKE_CROSSCOMPILING)
    option(MPAPP_BUILD_EXAMPLES "Build the platform example apps." ON)
else()
    option(MPAPP_BUILD_EXAMPLES "Build the platform example apps." OFF)
endif()

# 2D graphics backend per ADR-0015. The selection + detection logic lives in
# cmake/MpappGraphicsBackend.cmake (mpapp_select_graphics_backend); this only
# declares the option so it shows up in the cache / ccmake UI.
#
#   cairo  — LGPL via dynamic linking (RFC-0001). Default; auto-falls-back to
#            stub when libcairo isn't found via pkg-config.
#   stub   — records calls without rendering. No native dependency.
#   skia   — BSD-3, ~30 MB. Opt-in; pinned prebuilt fetched on first configure.
set(MPAPP_GRAPHICS_BACKEND "cairo" CACHE STRING
    "2D graphics backend: cairo (default; auto-falls-back to stub if not found), stub, or skia")
set_property(CACHE MPAPP_GRAPHICS_BACKEND PROPERTY STRINGS stub cairo skia)

# Opt-in quality gates (sanitizers / coverage / clang-tidy / -fanalyzer). Off by
# default; the quality presets switch them on. Global sanitizer + coverage
# instrumentation is applied here so the whole child build is consistent.
include(MpappHardening)
mpapp_init_hardening()
