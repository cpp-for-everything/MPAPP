# `mpapp` CLI — subcommand dispatcher (skeleton). Builds the CLI library, the
# executable, and a Catch2 dispatcher test.
include(MpappWarnings)
include(MpappTest)

add_library(mpapp-cli-lib STATIC
    src/commands.cpp
    src/process.cpp
)
target_include_directories(mpapp-cli-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(mpapp-cli-lib PUBLIC cxx_std_23)
mpapp_apply_warnings(mpapp-cli-lib)

add_executable(mpapp main.cpp)
target_link_libraries(mpapp PRIVATE mpapp-cli-lib)
mpapp_apply_warnings(mpapp)

if(BUILD_TESTING)
    mpapp_add_catch_test(mpapp_dispatch_test
        SOURCES tests/dispatch_test.cpp
        LINK    mpapp-cli-lib)
endif()
