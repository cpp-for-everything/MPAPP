# cmake/MpappHostCapabilities.cmake
#
# Detects what the SUPERBUILD orchestrator can drive on THIS host and produces
# the MPAPP_PLATFORMS list consumed by MpappSuperbuild.cmake. Included only in
# superbuild (orchestrator) mode — see the top-level CMakeLists.txt.
#
# MPAPP's cross substrate is Zig (ADR-0011): one `zig` install cross-compiles
# every target triple from any host with no per-platform SDK or WSL. So the
# "buildable" cross matrix is the same everywhere; we just confirm zig + ninja
# are present. The NATIVE host child (real WinUI 3 / GTK4 example apps) is a
# separate, host-specific concern toggled by MPAPP_HOST_NATIVE.
include_guard(GLOBAL)

# -- The full Zig cross matrix (compile-checks the portable core + tests) ------
# Override with -DMPAPP_PLATFORMS="linux-x64;windows-x64" to build a subset.
set(MPAPP_ALL_CROSS_PLATFORMS
    windows-x64
    windows-arm64
    linux-x64
    linux-arm64
    android-arm64
    macos-arm64
    ios-arm64)

option(MPAPP_HOST_NATIVE
    "Add a native host child that builds the example apps + tools with the host SDK" ON)

# -- Visual Studio generator for the native Windows child ----------------------
# The VS generator locates MSVC itself, so the superbuild runs from a plain
# shell (no vcvars) — that's what makes the WinUI 3 examples buildable without
# a Developer prompt. Override the version/arch if you run a different VS.
set(MPAPP_VS_GENERATOR "Visual Studio 17 2022"
    CACHE STRING "CMake generator for the native Windows host child")
set(MPAPP_VS_ARCH "x64" CACHE STRING "Architecture for the native Windows host child")

# -- Locate zig (same search order as cmake/toolchains/zig.cmake) --------------
set(MPAPP_ZIG_VERSION "0.13.0" CACHE STRING "Pinned Zig version for MPAPP cross-compilation")
if(NOT DEFINED MPAPP_ZIG_HOME)
    if(DEFINED ENV{MPAPP_ZIG_HOME})
        set(MPAPP_ZIG_HOME "$ENV{MPAPP_ZIG_HOME}")
    elseif(CMAKE_HOST_WIN32)
        set(MPAPP_ZIG_HOME "$ENV{USERPROFILE}/.mpapp/toolchains/zig-${MPAPP_ZIG_VERSION}")
    else()
        set(MPAPP_ZIG_HOME "$ENV{HOME}/.mpapp/toolchains/zig-${MPAPP_ZIG_VERSION}")
    endif()
endif()
find_program(MPAPP_ZIG NAMES zig zig.exe HINTS "${MPAPP_ZIG_HOME}")
find_program(MPAPP_NINJA NAMES ninja ninja.exe)

# -- Build the platform list when not explicitly provided ----------------------
if(NOT DEFINED MPAPP_PLATFORMS OR MPAPP_PLATFORMS STREQUAL "")
    if(MPAPP_ZIG AND MPAPP_NINJA)
        set(MPAPP_PLATFORMS "${MPAPP_ALL_CROSS_PLATFORMS}"
            CACHE STRING "Cross-compile targets this superbuild builds" FORCE)
    else()
        set(MPAPP_PLATFORMS "" CACHE STRING "Cross-compile targets this superbuild builds" FORCE)
    endif()
endif()

# -- Diagnostics ---------------------------------------------------------------
message(STATUS "MPAPP superbuild — host: ${CMAKE_HOST_SYSTEM_NAME}")
if(MPAPP_ZIG)
    message(STATUS "  zig:   ${MPAPP_ZIG}")
else()
    message(WARNING
        "MPAPP superbuild: zig not found (looked in ${MPAPP_ZIG_HOME} and PATH).\n"
        "  Cross-compile children are disabled. Install Zig ${MPAPP_ZIG_VERSION} or\n"
        "  run `mpapp build --target ...` once to auto-install it.")
endif()
if(MPAPP_NINJA)
    message(STATUS "  ninja: ${MPAPP_NINJA}")
elseif(MPAPP_ZIG)
    message(WARNING "MPAPP superbuild: ninja not found — cross-compile children are disabled.")
endif()
message(STATUS "  cross platforms: ${MPAPP_PLATFORMS}")
if(MPAPP_HOST_NATIVE)
    message(STATUS "  native host child: ON")
endif()

if(MPAPP_PLATFORMS STREQUAL "" AND NOT MPAPP_HOST_NATIVE)
    message(FATAL_ERROR
        "MPAPP superbuild: nothing to build — no cross platforms available and "
        "MPAPP_HOST_NATIVE is OFF.")
endif()
