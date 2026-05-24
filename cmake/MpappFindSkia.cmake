# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0030 — locate a Skia install in either of two layouts.
#
# Skia is opt-in (per ADR-0015). vcpkg's `unofficial-skia` port is the
# canonical install path on Windows/Linux because it ships a real CMake
# config package — but vcpkg's GN-driven Skia build hits two upstream
# tooling bugs when cross-compiling to Android from a Windows host
# (see vault/50_Tasks/T-0030-skia-backend/notes/dual-vcpkg-roots.md).
#
# To unstick that platform we also accept community **prebuilt** drops
# from HumbleUI/SkiaBuild and JetBrains/skia-pack. Both ship the same
# directory shape:
#
#   <prefix>/include/core/SkCanvas.h         ← Skia public headers
#   <prefix>/out/<config>/libskia.a          ← static archive (e.g.
#                                              Release-x64, Release-arm64)
#   <prefix>/out/<config>/lib<rest>.a        ← transitive deps
#                                              (libfreetype2.a, libicu.a,
#                                              libharfbuzz.a, ...)
#   <prefix>/out/<config>/defines.cmake      ← `add_definitions(-DSK_*...)`
#                                              generated from the actual
#                                              ninja invocation, so the
#                                              headers compile with the
#                                              same SK_* feature flags
#                                              the .a files were built
#                                              with
#
# `mpapp_find_skia()` tries the vcpkg layout first and falls back to the
# prebuilt layout. Either way it leaves the same imported target
# `unofficial::skia::skia` for the rest of the build to link.
#
# Inputs:
#   MPAPP_SKIA_PREFIX (optional) — install root. Added to
#                                  CMAKE_PREFIX_PATH for find_package
#                                  and probed directly for the prebuilt
#                                  layout.
#
# Outputs (set in the caller's scope — this is a macro, not a function):
#   MPAPP_SKIA_FOUND      ON when a usable install is located.
#   MPAPP_SKIA_LAYOUT     "vcpkg" | "prebuilt" | "none".
#   unofficial::skia::skia  imported INTERFACE target carrying include
#                           dirs + .a list + compile definitions.
#
# Implementation note: this is a `macro` rather than a `function` so the
# imported target lands in the caller's directory scope without needing
# the GLOBAL keyword — matches how `find_package()` normally behaves
# and keeps the call site uniform whether vcpkg or prebuilt won.

macro(mpapp_find_skia)
    set(MPAPP_SKIA_FOUND OFF)
    set(MPAPP_SKIA_LAYOUT "none")

    if(DEFINED MPAPP_SKIA_PREFIX)
        list(APPEND CMAKE_PREFIX_PATH "${MPAPP_SKIA_PREFIX}")
    endif()

    # Layout 1: vcpkg `unofficial-skia` CONFIG package.
    find_package(unofficial-skia CONFIG QUIET)
    if(TARGET unofficial::skia::skia)
        set(MPAPP_SKIA_FOUND ON)
        set(MPAPP_SKIA_LAYOUT "vcpkg")
    elseif(DEFINED MPAPP_SKIA_PREFIX)
        # Layout 2: HumbleUI/JetBrains prebuilt drop. Walk
        # <prefix>/out/* for a directory containing both libskia.a
        # and defines.cmake.
        set(_mpapp_skia_libdir "")
        if(IS_DIRECTORY "${MPAPP_SKIA_PREFIX}/out")
            file(GLOB _mpapp_skia_out_dirs
                 RELATIVE "${MPAPP_SKIA_PREFIX}/out"
                 "${MPAPP_SKIA_PREFIX}/out/*")
            foreach(_d IN LISTS _mpapp_skia_out_dirs)
                if(EXISTS "${MPAPP_SKIA_PREFIX}/out/${_d}/libskia.a"
                        AND EXISTS "${MPAPP_SKIA_PREFIX}/out/${_d}/defines.cmake")
                    set(_mpapp_skia_libdir "${MPAPP_SKIA_PREFIX}/out/${_d}")
                    break()
                endif()
            endforeach()
        endif()

        if(NOT _mpapp_skia_libdir STREQUAL "")
            # Enumerate every .a in the out/ dir. Put libskia first
            # because lld walks the list in order and the transitive
            # deps satisfy symbols requested by libskia.a (not the
            # other way around).
            file(GLOB _mpapp_skia_libs "${_mpapp_skia_libdir}/*.a")
            list(REMOVE_ITEM _mpapp_skia_libs
                 "${_mpapp_skia_libdir}/libskia.a")
            list(PREPEND _mpapp_skia_libs
                 "${_mpapp_skia_libdir}/libskia.a")

            # Pull the SK_* defines out of HumbleUI's generated
            # defines.cmake and attach them to the imported target.
            # The file looks like `add_definitions(-DFOO -DBAR=BAZ ...)`
            # — a regex extracts the tokens so we can stash them as
            # INTERFACE_COMPILE_DEFINITIONS (scoped to consumers of the
            # target) rather than spraying them project-wide.
            file(READ "${_mpapp_skia_libdir}/defines.cmake"
                 _mpapp_skia_defs_text)
            string(REGEX MATCHALL
                "-D[A-Za-z_][A-Za-z0-9_]*(=[^ \n\t)]+)?"
                _mpapp_skia_defs_list "${_mpapp_skia_defs_text}")
            list(TRANSFORM _mpapp_skia_defs_list REPLACE "^-D" "")

            add_library(unofficial::skia::skia INTERFACE IMPORTED)
            set_target_properties(unofficial::skia::skia PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${MPAPP_SKIA_PREFIX}"
                INTERFACE_LINK_LIBRARIES "${_mpapp_skia_libs}"
                INTERFACE_COMPILE_DEFINITIONS "${_mpapp_skia_defs_list}"
            )

            set(MPAPP_SKIA_FOUND ON)
            set(MPAPP_SKIA_LAYOUT "prebuilt")
        endif()
    endif()
endmacro()
