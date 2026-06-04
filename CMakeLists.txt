# MPAPP — C++ cross-platform UI framework.
#
# Two modes, selected by MPAPP_SUPERBUILD (default ON):
#
#   SUPERBUILD (orchestrator) — `cmake -S . -B build && cmake --build build`
#     configures + builds every platform this host supports, each into
#     build/<platform>/. Cross targets go through Zig (cmake/toolchains/*),
#     the native host child builds the example apps with the platform SDK.
#     All the orchestration lives in cmake/MpappSuperbuild.cmake.
#
#   CHILD (MPAPP_SUPERBUILD=OFF) — the real build of mpapp-core + tests +
#     tools + examples for ONE platform (native, or cross via a toolchain
#     file). This is what the orchestrator invokes per platform; you can also
#     invoke it directly with -DMPAPP_SUPERBUILD=OFF.
#
# Build logic is factored into cmake/ helpers; the per-directory CMakeLists in
# src/, tests/, tools/, and examples/ stay clean target declarations.
cmake_minimum_required(VERSION 3.28)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(MPAPP_SUPERBUILD
    "Top-level orchestrator: one build dir builds every host-supported platform" ON)

# ---- SUPERBUILD: orchestrate per-platform child builds -----------------------
if(MPAPP_SUPERBUILD)
    project(mpapp LANGUAGES NONE)
    include(MpappHostCapabilities)   # -> MPAPP_PLATFORMS, MPAPP_HOST_NATIVE
    include(MpappSuperbuild)
    mpapp_setup_platform_children()  # config/build target per platform + mpapp-all (ALL)
    return()
endif()

# ---- CHILD: real native/cross build for one platform ------------------------
project(mpapp
    VERSION 0.0.1
    DESCRIPTION "MPAPP - C++ cross-platform UI framework"
    LANGUAGES CXX
)

include(MpappOptions)   # C++23 baseline, warning/EH workarounds, build options

# The portable library (mpapp-core) + the per-platform handler libs.
add_subdirectory(src)

# Tests — initialized before any other subdirectory so nested CMakeLists can
# observe BUILD_TESTING and register Catch2 targets.
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Host-platform developer tools (mpapp CLI, mpapp-xc). Added after tests so the
# tool test binaries can reuse the Catch2 target the tests subdirectory fetched.
if(MPAPP_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

# Platform example apps (WinUI 3 / GTK4 spikes, headless demos).
if(MPAPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
