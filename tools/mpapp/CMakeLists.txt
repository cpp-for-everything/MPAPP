# `mpapp` CLI — skeleton.
#
# Subcommand dispatcher with stub implementations. Future tasks fill in the
# behavior of each subcommand; this target builds the executable plus a
# Catch2 unit-test binary for the dispatcher itself.

add_library(mpapp-cli-lib STATIC
    src/commands.cpp
    src/process.cpp
)

target_include_directories(mpapp-cli-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(mpapp-cli-lib PUBLIC cxx_std_23)

target_compile_options(mpapp-cli-lib PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
)

add_executable(mpapp main.cpp)
target_link_libraries(mpapp PRIVATE mpapp-cli-lib)

target_compile_options(mpapp PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
)

if(BUILD_TESTING)
    add_executable(mpapp_dispatch_test tests/dispatch_test.cpp)
    target_link_libraries(mpapp_dispatch_test PRIVATE
        mpapp-cli-lib
        Catch2::Catch2WithMain
    )
    target_compile_options(mpapp_dispatch_test PRIVATE
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
        $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
    )
    catch_discover_tests(mpapp_dispatch_test)
endif()
