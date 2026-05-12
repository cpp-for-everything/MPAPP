cmake_minimum_required(VERSION 3.28)

project(mpapp
    VERSION 0.0.1
    DESCRIPTION "MPAPP - C++ cross-platform UI framework"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Host-platform tools (mpapp CLI, etc.) are off by default for cross-compiles
# and on otherwise. CMAKE_CROSSCOMPILING is set by toolchain files; when it is
# truthy we default the option off so a target build does not also pull in the
# host CLI. The option remains overridable from the command line either way.
if(CMAKE_CROSSCOMPILING)
    set(MPAPP_BUILD_TOOLS_DEFAULT OFF)
else()
    set(MPAPP_BUILD_TOOLS_DEFAULT ON)
endif()
option(MPAPP_BUILD_TOOLS
    "Build host-platform tools (the mpapp CLI and friends)."
    ${MPAPP_BUILD_TOOLS_DEFAULT}
)

# mpapp-core static library
add_library(mpapp-core STATIC
    src/mpapp.cpp
)

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
