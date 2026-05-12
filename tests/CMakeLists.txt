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
