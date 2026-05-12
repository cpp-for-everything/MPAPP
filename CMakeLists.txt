cmake_minimum_required(VERSION 3.28)

project(mpapp
    VERSION 0.0.1
    DESCRIPTION "MPAPP - C++ cross-platform UI framework"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Examples are built by default on Windows hosts (where WinUI 3 ships),
# off elsewhere. The Linux/macOS spikes get the same default-on treatment
# once their build prereqs land — see T-0007, T-0008.
if(WIN32)
    option(MPAPP_BUILD_EXAMPLES "Build the platform example apps." ON)
else()
    option(MPAPP_BUILD_EXAMPLES "Build the platform example apps." OFF)
endif()

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

# Testing
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Platform-specific example apps (WinUI 3, GTK4, etc.).
if(MPAPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
