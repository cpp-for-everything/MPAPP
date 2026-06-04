# cmake/MpappGraphicsBackend.cmake
#
# Selects + wires the 2D graphics backend (ADR-0015) onto a target — normally
# mpapp-core. Was ~110 lines inline in the root CMakeLists.txt.
#
#   mpapp_select_graphics_backend(<target>)
#       Adds the chosen backend .cpp to <target> and wires its include dirs,
#       link libs, and feature-test defines. Honours the MPAPP_GRAPHICS_BACKEND
#       cache option (cairo / stub / skia) declared in MpappOptions.cmake.
#       Falls back to the stub backend (with a warning) when cairo/skia aren't
#       found, so the build always stays green.
#
# Side effects: sets the cache-internal flags MPAPP_GRAPHICS_HAS_CAIRO /
# MPAPP_GRAPHICS_HAS_SKIA so downstream targets (e.g. the headless canvas demo)
# can branch on what actually got linked.
include_guard(GLOBAL)

function(mpapp_select_graphics_backend target)
    set(_gfx "${CMAKE_SOURCE_DIR}/src/detail/graphics")
    set(_has_cairo OFF)
    set(_has_skia  OFF)

    if(MPAPP_GRAPHICS_BACKEND STREQUAL "cairo")
        # pkg-config is the detection path on all platforms. vcpkg ships cairo
        # with .pc files and bundles mingw64 pkgconf; _build_full.bat wires
        # PKG_CONFIG_EXECUTABLE to it so this same logic works on any host.
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(MPAPP_CAIRO QUIET cairo)
        endif()
        if(MPAPP_CAIRO_FOUND)
            target_sources(${target} PRIVATE "${_gfx}/cairo_backend.cpp")
            target_include_directories(${target} PUBLIC ${MPAPP_CAIRO_INCLUDE_DIRS})
            target_link_libraries(${target} PUBLIC ${MPAPP_CAIRO_LIBRARIES})
            target_link_directories(${target} PUBLIC ${MPAPP_CAIRO_LIBRARY_DIRS})
            target_compile_definitions(${target} PUBLIC MPAPP_GRAPHICS_HAS_CAIRO=1)
            set(_has_cairo ON)
            message(STATUS "MPAPP graphics backend: cairo (${MPAPP_CAIRO_VERSION})")
        else()
            target_sources(${target} PRIVATE "${_gfx}/stub_backend.cpp")
            message(WARNING
                "MPAPP_GRAPHICS_BACKEND=cairo selected but libcairo not found via "
                "pkg-config. Falling back to stub backend.\n"
                "  Linux:   apt install libcairo2-dev pkg-config\n"
                "  Windows: vcpkg install cairo:x64-windows, then pass\n"
                "             -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake\n"
                "             -DPKG_CONFIG_EXECUTABLE=<vcpkg>/downloads/tools/msys2/.../mingw64/bin/pkgconf.exe")
        endif()
    elseif(MPAPP_GRAPHICS_BACKEND STREQUAL "stub")
        target_sources(${target} PRIVATE "${_gfx}/stub_backend.cpp")
        message(STATUS "MPAPP graphics backend: stub")
    elseif(MPAPP_GRAPHICS_BACKEND STREQUAL "skia")
        # Skia is heavyweight (BSD-3, ~30 MB) and opt-in. MpappFindSkia
        # downloads the pinned HumbleUI prebuilt via FetchContent on first
        # configure (override with -DMPAPP_SKIA_PREFIX=<path>). Falls back to
        # stub when no prebuilt exists for the target.
        include(MpappFindSkia)
        mpapp_find_skia()
        if(MPAPP_SKIA_FOUND)
            target_sources(${target} PRIVATE "${_gfx}/skia_backend.cpp")
            target_link_libraries(${target} PUBLIC unofficial::skia::skia)
            target_compile_definitions(${target} PUBLIC MPAPP_GRAPHICS_HAS_SKIA=1)
            set(_has_skia ON)
            message(STATUS "MPAPP graphics backend: skia (${MPAPP_SKIA_LAYOUT})")
        else()
            target_sources(${target} PRIVATE "${_gfx}/stub_backend.cpp")
            message(WARNING
                "MPAPP_GRAPHICS_BACKEND=skia selected but Skia not found for "
                "CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}, "
                "CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}.\n"
                "  Set -DMPAPP_SKIA_PREFIX=<path> or add a row to "
                "cmake/MpappFindSkia.cmake. Falling back to stub backend.")
        endif()
    else()
        message(FATAL_ERROR
            "Unknown MPAPP_GRAPHICS_BACKEND='${MPAPP_GRAPHICS_BACKEND}'. "
            "Expected stub / cairo / skia.")
    endif()

    # Publish what actually linked so other targets can branch on it.
    set(MPAPP_GRAPHICS_HAS_CAIRO ${_has_cairo} CACHE INTERNAL "Cairo backend linked into mpapp-core")
    set(MPAPP_GRAPHICS_HAS_SKIA  ${_has_skia}  CACHE INTERNAL "Skia backend linked into mpapp-core")
endfunction()
