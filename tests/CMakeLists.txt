include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

include(Catch)

add_executable(smoke_test smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE mpapp-core Catch2::Catch2WithMain)
catch_discover_tests(smoke_test)

# Async executor skeleton — task<T>, ui_task<T>, test_dispatcher.
add_executable(executor_test executor_test.cpp)
target_link_libraries(executor_test PRIVATE mpapp-core Catch2::Catch2WithMain)
target_compile_options(executor_test PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
)
catch_discover_tests(executor_test)

add_subdirectory(template_type_spike)

# Mock-handler unit tests for the P2 component surface (ADR-0008).
# Layout-group and simple-input handlers share a single Catch2 executable —
# they pull in the same handler-base infrastructure and exercise the same
# recording contract, so a combined binary keeps build cost low.
#
# Per ADR-0013 the test list is a glob — adding a new widget's
# `<widget>_test.cpp` doesn't require touching this file.
file(GLOB MPAPP_MOCK_TESTS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/mock_handlers/*_test.cpp
)
add_executable(mock_handlers_test
    ${MPAPP_MOCK_TESTS}
)
target_link_libraries(mock_handlers_test PRIVATE mpapp-core Catch2::Catch2WithMain)
target_compile_options(mock_handlers_test PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
)
catch_discover_tests(mock_handlers_test)

# T-0010 hot-reload spike — Windows-only.
if(WIN32)
    add_executable(hot_reload_test
        ${CMAKE_SOURCE_DIR}/vault/50_Tasks/_Archive/T-0010-hot-reload-spike/tests/hot_reload_test.cpp
    )
    target_link_libraries(hot_reload_test PRIVATE mpapp-core Catch2::Catch2WithMain)
    target_compile_options(hot_reload_test PRIVATE
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
        $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
    )
    catch_discover_tests(hot_reload_test)
endif()
