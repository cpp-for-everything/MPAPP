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

add_subdirectory(template_type_spike)

# Mock-handler tests for the P2 layout/primitive group (ADR-0008).
# All seven components share a single Catch2 executable — they pull in
# the same handler-base infrastructure and exercise the same recording
# contract, so a combined binary keeps build cost low.
add_executable(mock_handlers_test
    mock_handlers/view_test.cpp
    mock_handlers/layout_test.cpp
    mock_handlers/bindable_layout_test.cpp
    mock_handlers/scroll_view_test.cpp
    mock_handlers/border_test.cpp
    mock_handlers/frame_test.cpp
    mock_handlers/box_view_test.cpp
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
        ${CMAKE_SOURCE_DIR}/vault/50_Tasks/T-0010-hot-reload-spike/tests/hot_reload_test.cpp
    )
    target_link_libraries(hot_reload_test PRIVATE mpapp-core Catch2::Catch2WithMain)
    target_compile_options(hot_reload_test PRIVATE
        $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
        $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
    )
    catch_discover_tests(hot_reload_test)
endif()
