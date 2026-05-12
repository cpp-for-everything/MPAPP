cmake_minimum_required(VERSION 3.28)

project(mpapp
    VERSION 0.0.1
    DESCRIPTION "MPAPP - C++ cross-platform UI framework"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MPAPP_BUILD_EXAMPLES "Build MPAPP example programs" ON)

# mpapp-core static library
set(MPAPP_CORE_SOURCES src/mpapp.cpp)

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

# Examples (Windows-only spikes today; see examples/CMakeLists.txt).
if(MPAPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Testing
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
