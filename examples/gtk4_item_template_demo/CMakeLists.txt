# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0019 — CollectionView item_template demo (Linux/GTK4).
#
# Visible exercise of `collection_view::item_template` —
# `std::function<unique_ptr<view>(int)>` factory that materializes one
# typed cell per items_source row.

if(NOT (UNIX AND NOT APPLE))
    return()
endif()

add_executable(gtk4_item_template_demo main.cpp)

target_link_libraries(gtk4_item_template_demo PRIVATE
    mpapp-core
    mpapp-handlers-linux
)

target_compile_features(gtk4_item_template_demo PRIVATE cxx_std_23)
target_compile_options(gtk4_item_template_demo PRIVATE
    -Wall -Wextra -Wpedantic
)
