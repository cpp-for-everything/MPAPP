cmake_minimum_required(VERSION 3.28)

project(mpapp
    VERSION 0.0.1
    DESCRIPTION "MPAPP - C++ cross-platform UI framework"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
# ship — currently only Windows (WinUI 3, the hot-reload spike). Linux/macOS
# spikes get the same default-on treatment as their build prereqs land
# (see T-0007 WSLg, T-0008 macOS).
if(WIN32)
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

if(WIN32)
    list(APPEND MPAPP_CORE_SOURCES src/hot_reload/windows.cpp)
endif()

add_library(mpapp-core STATIC ${MPAPP_CORE_SOURCES})

target_include_directories(mpapp-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

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
