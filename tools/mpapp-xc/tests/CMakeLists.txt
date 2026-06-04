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

# Look up Catch2's source directory regardless of who fetched it. The
# `catch2_SOURCE_DIR` variable is only set in the scope that called
# FetchContent_MakeAvailable; FetchContent_GetProperties works across scopes.
FetchContent_GetProperties(Catch2 SOURCE_DIR _mpapp_xc_catch2_src)
list(APPEND CMAKE_MODULE_PATH "${_mpapp_xc_catch2_src}/extras")
include(Catch)

set(MPAPP_XC_FIXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
set(MPAPP_XC_EXE "$<TARGET_FILE:mpapp-xc>")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/test_paths.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/test_paths.hpp.tmp"
    @ONLY
)

# Use file(GENERATE) so MPAPP_XC_EXE's generator expression is expanded
# correctly per-config. The output is written under a `$<CONFIG>/`
# subdirectory so multi-config generators (Visual Studio) can keep one
# copy per config without "Evaluation file to be written multiple times
# with different content" — `$<TARGET_FILE:mpapp-xc>` resolves to a
# different absolute path under Release/ vs Debug/ vs RelWithDebInfo/.
# Single-config generators (Ninja, Makefiles) just write to
# `<binary-dir>/Release/test_paths.hpp` and life carries on.
file(GENERATE
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/test_paths.hpp"
    INPUT  "${CMAKE_CURRENT_BINARY_DIR}/test_paths.hpp.tmp"
)

add_executable(mpapp_xc_empty_xaml_test empty_xaml_test.cpp)
target_link_libraries(mpapp_xc_empty_xaml_test PRIVATE mpapp-xc-lib Catch2::Catch2WithMain)
target_include_directories(mpapp_xc_empty_xaml_test PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
)
add_dependencies(mpapp_xc_empty_xaml_test mpapp-xc)

catch_discover_tests(mpapp_xc_empty_xaml_test)

add_executable(mpapp_xc_lowering_test lowering_test.cpp)
target_link_libraries(mpapp_xc_lowering_test PRIVATE mpapp-xc-lib Catch2::Catch2WithMain)
target_include_directories(mpapp_xc_lowering_test PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
)
add_dependencies(mpapp_xc_lowering_test mpapp-xc)

catch_discover_tests(mpapp_xc_lowering_test)
