# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0017 — typed routing demo (Linux/GTK4).
#
# Visible exercise of ADR-0016 compile-time `route_table` +
# `shell::go_to<Path, &Table>(args...)` + ADR-0023 route guards +
# page lifecycle hooks.

if(NOT (UNIX AND NOT APPLE))
    return()
endif()

# Reuse mpapp-handlers-linux that gtk4_hello already builds.
add_executable(gtk4_routes_demo main.cpp)

target_link_libraries(gtk4_routes_demo PRIVATE
    mpapp-core
    mpapp-handlers-linux
)

target_compile_features(gtk4_routes_demo PRIVATE cxx_std_23)
target_compile_options(gtk4_routes_demo PRIVATE
    -Wall -Wextra -Wpedantic
)
