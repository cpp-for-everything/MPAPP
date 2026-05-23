# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0029 / T-0031 closure evidence — renders the
# canvas-facade pipelines under test directly to PNG so Rule 11
# evidence doesn't depend on a running GUI / screen capture.
#
# Gated on Cairo because cairo_surface_write_to_png is the easiest
# cross-host PNG writer to reach; the rendering pipeline under test
# IS the abstract canvas facade, so any backend (Cairo today, Skia
# tomorrow) produces the same PNG.

cmake_minimum_required(VERSION 3.28)

if(NOT MPAPP_GRAPHICS_HAS_CAIRO)
    return()
endif()

add_executable(headless_canvas_demo main.cpp)
target_link_libraries(headless_canvas_demo PRIVATE mpapp-core)
target_compile_features(headless_canvas_demo PRIVATE cxx_std_23)
