# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0029 / T-0031 closure evidence — renders the
# canvas-facade pipelines under test directly to PNG so Rule 11
# evidence doesn't depend on a running GUI / screen capture.
#
# The rendering pipeline under test IS the abstract canvas facade —
# any backend (Cairo, Skia, stub) produces the same observable
# output. The demo uses cairo_surface_write_to_png as its PNG sink
# regardless of which canvas backend is selected, so cairo must be
# findable here even when the backend is Skia. We find it directly
# rather than relying on mpapp-core's transitive link because
# MPAPP_GRAPHICS_HAS_CAIRO is OFF in non-Cairo configs.

cmake_minimum_required(VERSION 3.28)

set(_demo_cairo_includes "")
set(_demo_cairo_libs     "")
set(_demo_cairo_libdirs  "")
if(MPAPP_GRAPHICS_HAS_CAIRO)
    # mpapp-core already links cairo — inherit its include path / libs
    # from there. (PUBLIC link from mpapp-core means linking it
    # transitively brings cairo in too.)
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(DEMO_CAIRO QUIET cairo)
    endif()
    if(NOT DEMO_CAIRO_FOUND)
        message(STATUS
            "headless_canvas_demo: skipping — needs libcairo for PNG output, "
            "not found via pkg-config and not pulled in transitively by mpapp-core.")
        return()
    endif()
    set(_demo_cairo_includes ${DEMO_CAIRO_INCLUDE_DIRS})
    set(_demo_cairo_libs     ${DEMO_CAIRO_LIBRARIES})
    set(_demo_cairo_libdirs  ${DEMO_CAIRO_LIBRARY_DIRS})
endif()

add_executable(headless_canvas_demo main.cpp)
target_link_libraries(headless_canvas_demo PRIVATE mpapp-core ${_demo_cairo_libs})
if(_demo_cairo_includes)
    target_include_directories(headless_canvas_demo PRIVATE ${_demo_cairo_includes})
endif()
if(_demo_cairo_libdirs)
    # pkg-config returns bare library names (e.g. "cairo"); the linker
    # needs the lib directory on its search path to resolve them.
    target_link_directories(headless_canvas_demo PRIVATE ${_demo_cairo_libdirs})
endif()
target_compile_features(headless_canvas_demo PRIVATE cxx_std_23)
