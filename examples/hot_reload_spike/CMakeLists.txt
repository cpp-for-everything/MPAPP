# T-0010 hot-reload spike — desktop host (Windows + Linux). Links mpapp-core,
# which pulls in the hot_reload runtime. user_code.cpp is NOT built by CMake;
# the host recompiles it at run time, so it's staged next to the executable.
mpapp_add_example(hot_reload_spike_host SOURCES host.cpp STRICT)

add_custom_command(TARGET hot_reload_spike_host POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/user_code.cpp"
            "$<TARGET_FILE_DIR:hot_reload_spike_host>/user_code.cpp"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/user_code.h"
            "$<TARGET_FILE_DIR:hot_reload_spike_host>/user_code.h"
    COMMENT "Staging user_code.{cpp,h} next to hot_reload_spike_host")
