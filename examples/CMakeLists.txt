# examples/CMakeLists.txt — one add_subdirectory per example app.
#
# The per-platform handler library is built ONCE here (cmake/MpappHandlers.cmake)
# and every example links the shared mpapp-handlers-<platform> target. Each
# example's own CMakeLists.txt is a one-line recipe via the helpers in
# cmake/MpappExample.cmake.
include(MpappExample)
include(MpappHandlers)

if(WIN32)
    mpapp_add_handler_library(windows)

    add_subdirectory(hot_reload_spike)
    add_subdirectory(windows_button_spike)
    add_subdirectory(windows_nav_spike)
    add_subdirectory(windows_tableview_demo)
    add_subdirectory(windows_routes_demo)
    add_subdirectory(windows_async_bridge_demo)
    add_subdirectory(windows_item_template_demo)
    add_subdirectory(windows_listview_demo)
    add_subdirectory(windows_collectionview_layout_demo)
    add_subdirectory(windows_shapeview_demo)
    add_subdirectory(windows_graphicsview_demo)
    add_subdirectory(windows_tabbed_demo)
    add_subdirectory(windows_flyout_demo)
    add_subdirectory(windows_contentpage_demo)
    add_subdirectory(windows_webview_demo)
    add_subdirectory(windows_hybridwebview_demo)
    add_subdirectory(windows_vsm_demo)
    add_subdirectory(uiss)
endif()

if(UNIX AND NOT APPLE)
    mpapp_add_handler_library(linux)

    add_subdirectory(gtk4_hello)
    add_subdirectory(gtk4_tableview_demo)
    add_subdirectory(gtk4_routes_demo)
    add_subdirectory(gtk4_async_bridge_demo)
    add_subdirectory(gtk4_item_template_demo)
    add_subdirectory(gtk4_listview_demo)
    add_subdirectory(gtk4_collectionview_layout_demo)
    add_subdirectory(gtk4_shapeview_demo)
    add_subdirectory(gtk4_graphicsview_demo)
    add_subdirectory(gtk4_tabbed_demo)
    add_subdirectory(gtk4_flyout_demo)
    add_subdirectory(gtk4_contentpage_demo)
    add_subdirectory(gtk4_webview_demo)
    add_subdirectory(gtk4_hybridwebview_demo)
    add_subdirectory(gtk4_vsm_demo)
    add_subdirectory(uiss)
    add_subdirectory(hot_reload_spike)
endif()

# Cross-platform demos of the canvas facade (ADR-0015) — render to PNG, no GUI.
add_subdirectory(cairo_render_demo)
add_subdirectory(headless_canvas_demo)
