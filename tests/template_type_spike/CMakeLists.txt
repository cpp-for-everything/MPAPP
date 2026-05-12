cmake_minimum_required(VERSION 3.24)
project(mpapp_template_type_spike LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.6.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

add_executable(template_type_spike_test test.cpp)

# Repo-root-relative include path: the spike lives at
# tests/template_type_spike/ and headers live at include/mpapp/.
target_include_directories(template_type_spike_test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
)

target_link_libraries(template_type_spike_test PRIVATE Catch2::Catch2WithMain)

if(MSVC)
    target_compile_options(template_type_spike_test PRIVATE /W4 /permissive-)
else()
    target_compile_options(template_type_spike_test PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()

enable_testing()
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(template_type_spike_test)
