# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0024 — FlyoutPage demo (Linux/GTK4).

if(NOT (UNIX AND NOT APPLE))
    return()
endif()

add_executable(gtk4_flyout_demo main.cpp)

target_link_libraries(gtk4_flyout_demo PRIVATE
    mpapp-core
    mpapp-handlers-linux
)

target_compile_features(gtk4_flyout_demo PRIVATE cxx_std_23)
target_compile_options(gtk4_flyout_demo PRIVATE
    -Wall -Wextra -Wpedantic
)
