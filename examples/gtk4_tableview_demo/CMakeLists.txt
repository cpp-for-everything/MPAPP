# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. TableView typed_sections demo — builds a TableView
# whose rows are real cell-tree instances (text_cell + entry_cell +
# switch_cell) and verifies the ADR-0021 cell tree composes end-to-end
# with the ADR-0013 dispatch on Linux.

if(NOT (UNIX AND NOT APPLE))
    return()
endif()

# The gtk4_hello example already builds mpapp-handlers-linux out of
# src/handlers/linux/*.cpp via glob + links GTK4 + WebKitGTK. Reuse
# that target.

add_executable(gtk4_tableview_demo main.cpp)

target_link_libraries(gtk4_tableview_demo PRIVATE
    mpapp-core
    mpapp-handlers-linux
)

target_compile_features(gtk4_tableview_demo PRIVATE cxx_std_23)
target_compile_options(gtk4_tableview_demo PRIVATE
    -Wall -Wextra -Wpedantic
)
