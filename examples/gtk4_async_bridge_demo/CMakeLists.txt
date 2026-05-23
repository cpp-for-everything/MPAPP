# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0018 — async bridge dispatch demo (Linux/GTK4).
#
# Visible exercise of ADR-0018 Phase F — `hybrid_bridge::dispatch_async`
# routing sync methods + inline-responding async methods + deferred-
# respond async methods.

if(NOT (UNIX AND NOT APPLE))
    return()
endif()

# Reuse mpapp-handlers-linux that gtk4_hello already builds.
add_executable(gtk4_async_bridge_demo main.cpp)

target_link_libraries(gtk4_async_bridge_demo PRIVATE
    mpapp-core
    mpapp-handlers-linux
)

target_compile_features(gtk4_async_bridge_demo PRIVATE cxx_std_23)
target_compile_options(gtk4_async_bridge_demo PRIVATE
    -Wall -Wextra -Wpedantic
)
