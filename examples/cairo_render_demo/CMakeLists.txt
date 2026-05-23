# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. Visible Cairo backend demo per [[ADR-0015]].

cmake_minimum_required(VERSION 3.28)

add_executable(cairo_render_demo cairo_render_demo.cpp)
target_link_libraries(cairo_render_demo PRIVATE mpapp-core)
target_compile_features(cairo_render_demo PRIVATE cxx_std_23)

# The demo uses <cairo/cairo.h> when the Cairo backend is selected,
# guarded by `#if defined(MPAPP_GRAPHICS_HAS_CAIRO)`. The compile
# definition + include path propagate from mpapp-core (set in the
# root CMakeLists.txt when Cairo is detected).
