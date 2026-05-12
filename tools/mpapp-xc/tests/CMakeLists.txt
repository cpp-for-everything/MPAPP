include(FetchContent)

# Reuse Catch2 if the parent already declared it (the root tests/ directory
# does), otherwise pull it ourselves so this tree can build standalone.
if(NOT TARGET Catch2::Catch2WithMain)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.4
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(Catch2)
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

set(MPAPP_XC_FIXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
set(MPAPP_XC_EXE "$<TARGET_FILE:mpapp-xc>")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/test_paths.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/test_paths.hpp.tmp"
    @ONLY
)

# Use file(GENERATE) so MPAPP_XC_EXE's generator expression is expanded
# correctly per-config.
file(GENERATE
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test_paths.hpp"
    INPUT  "${CMAKE_CURRENT_BINARY_DIR}/test_paths.hpp.tmp"
)

add_executable(mpapp_xc_empty_xaml_test empty_xaml_test.cpp)
target_link_libraries(mpapp_xc_empty_xaml_test PRIVATE mpapp-xc-lib Catch2::Catch2WithMain)
target_include_directories(mpapp_xc_empty_xaml_test PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
)
add_dependencies(mpapp_xc_empty_xaml_test mpapp-xc)

catch_discover_tests(mpapp_xc_empty_xaml_test)
