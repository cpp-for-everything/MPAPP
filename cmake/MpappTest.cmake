# cmake/MpappTest.cmake
#
# Catch2 provisioning + a one-line test helper, so tests/CMakeLists.txt (and
# the tool test dirs) just declare test binaries.
#
#   mpapp_enable_catch2()
#       Fetches Catch2 v3 (once) and makes catch_discover_tests available.
#       Call before any mpapp_add_catch_test().
#
#   mpapp_add_catch_test(<name> SOURCES <...> [LINK <...>])
#       Builds a Catch2 test exe, applies strict warnings, and registers it
#       with CTest. LINK defaults to mpapp-core; pass LINK to override (e.g.
#       the tool tests link mpapp-cli-lib instead).
include_guard(GLOBAL)

include(MpappWarnings)

set(MPAPP_CATCH2_VERSION "v3.5.4" CACHE STRING "Pinned Catch2 version")

function(mpapp_enable_catch2)
    if(TARGET Catch2::Catch2WithMain)
        return()
    endif()
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        ${MPAPP_CATCH2_VERSION}
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
    set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" PARENT_SCOPE)
    include(Catch)
endfunction()

function(mpapp_add_catch_test name)
    cmake_parse_arguments(A "" "" "SOURCES;LINK" ${ARGN})
    if(NOT A_LINK)
        set(A_LINK mpapp-core)
    endif()
    add_executable(${name} ${A_SOURCES})
    target_link_libraries(${name} PRIVATE ${A_LINK} Catch2::Catch2WithMain)
    mpapp_apply_warnings(${name})
    catch_discover_tests(${name})
endfunction()
