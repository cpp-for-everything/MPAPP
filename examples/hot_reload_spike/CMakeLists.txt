# T-0010 hot-reload spike — desktop host program (Windows + Linux).
#
# The host links mpapp-core which pulls in the hot_reload runtime via
# src/hot_reload/{windows,linux}.cpp. user_code.cpp is NOT built by CMake;
# the host's runtime invokes the compiler at run time to produce a sibling
# user_code.{dll,so}. user_code.cpp is copied into the build directory so
# the host can find and recompile it from the default cwd.

if(NOT WIN32 AND NOT (UNIX AND NOT APPLE))
    return()
endif()

add_executable(hot_reload_spike_host host.cpp)
target_link_libraries(hot_reload_spike_host PRIVATE mpapp-core)

target_compile_options(hot_reload_spike_host PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
)

# Stage user_code.cpp + user_code.h next to the host so it can be edited
# and recompiled from the host's default working directory.
add_custom_command(TARGET hot_reload_spike_host POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/user_code.cpp"
            "$<TARGET_FILE_DIR:hot_reload_spike_host>/user_code.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/user_code.h"
            "$<TARGET_FILE_DIR:hot_reload_spike_host>/user_code.h"
    COMMENT "Staging user_code.{cpp,h} next to hot_reload_spike_host"
)
