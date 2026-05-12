# MPAPP examples — gated on MPAPP_BUILD_EXAMPLES from the root CMakeLists.
#
# Each subdirectory is responsible for gating itself on the platforms it
# supports. The hot-reload spike is Windows-only.

if(WIN32)
    add_subdirectory(hot_reload_spike)
endif()
