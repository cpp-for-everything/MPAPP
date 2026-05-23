# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0020 — ListView demo (Linux/GTK4).

if(NOT (UNIX AND NOT APPLE))
    return()
endif()

add_executable(gtk4_listview_demo main.cpp)

target_link_libraries(gtk4_listview_demo PRIVATE
    mpapp-core
    mpapp-handlers-linux
)

target_compile_features(gtk4_listview_demo PRIVATE cxx_std_23)
target_compile_options(gtk4_listview_demo PRIVATE
    -Wall -Wextra -Wpedantic
)
