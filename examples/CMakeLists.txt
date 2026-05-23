# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. Examples — one subdirectory per platform-specific spike.
#
# Each subdirectory is responsible for gating itself on the platforms it
# supports. Windows hosts the WinUI 3 spikes; Linux (incl. WSLg from a
# Windows host) hosts the GTK4 hello-window smoke target.

if(WIN32)
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
endif()

if(UNIX AND NOT APPLE)
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
endif()

# Cross-platform CLI demo of the canvas facade (ADR-0015). Renders
# shapes to a PNG via the active backend; visible end-to-end proof
# that the Cairo backend wiring is correct on each host.
add_subdirectory(cairo_render_demo)

# Closure evidence for T-0029 (graphics_view drawable through canvas
# facade) + T-0031 (shape_view via shared render_shape_view). Headless
# — writes PNGs to disk so Rule 11 doesn't depend on a live GUI / screen
# capture. Self-gates on MPAPP_GRAPHICS_HAS_CAIRO inside its
# CMakeLists.
add_subdirectory(headless_canvas_demo)
