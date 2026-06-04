# cmake/MpappSuperbuild.cmake
#
# The one-configure multi-platform orchestrator (MPAPP's take on the MAUI++
# superbuild idea, simplified by Zig). In superbuild mode:
#
#     cmake -S . -B build         # configure the orchestrator
#     cmake --build build         # builds EVERY host-supported platform
#
# `mpapp_setup_platform_children()` emits, per platform, a config + build pair
# of custom targets that re-invoke CMake on this same source tree with
# MPAPP_SUPERBUILD=OFF (a "child") into build/<platform>/:
#
#   * Cross children use a Zig toolchain file (cmake/toolchains/<p>.cmake) and
#     build the portable mpapp-core + tests for that triple. No SDK/WSL/vcvars
#     — this is what lets the superbuild run out-of-the-box on any host.
#   * The native host child uses the host compiler (the VS generator on
#     Windows, Ninja elsewhere) and builds the real example apps + tools with
#     the platform SDK (WinUI 3 / GTK4).
#
# An aggregate `mpapp-all` (ALL) target depends on every child build, so a bare
# `cmake --build build` does the whole matrix.
include_guard(GLOBAL)

function(mpapp_setup_platform_children)
    set(_tc "${CMAKE_SOURCE_DIR}/cmake/toolchains")
    set(_build_all_deps "")

    # Single-config generator for the cross children + non-Windows native child.
    if(MPAPP_NINJA)
        set(_cross_gen "Ninja")
    else()
        set(_cross_gen "")  # let CMake pick the platform default
    endif()

    # ---- native host child: real example apps + tools with the host SDK ------
    if(MPAPP_HOST_NATIVE)
        set(_bin "${CMAKE_BINARY_DIR}/host")
        if(CMAKE_HOST_WIN32)
            # VS generator finds MSVC on its own (no vcvars), which the WinUI 3
            # examples need. Multi-config → choose the config at build time.
            add_custom_target(mpapp-config-host
                COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}" -B "${_bin}"
                        -G "${MPAPP_VS_GENERATOR}" -A "${MPAPP_VS_ARCH}"
                        -DMPAPP_SUPERBUILD=OFF
                COMMENT "[mpapp] configure native host child (${MPAPP_VS_GENERATOR} ${MPAPP_VS_ARCH})"
                USES_TERMINAL VERBATIM)
            add_custom_target(mpapp-build-host
                COMMAND ${CMAKE_COMMAND} --build "${_bin}" --config Debug
                COMMENT "[mpapp] build native host child" USES_TERMINAL VERBATIM)
        else()
            _mpapp_child_config_target(mpapp-config-host "${_bin}" "" "native host")
            add_custom_target(mpapp-build-host
                COMMAND ${CMAKE_COMMAND} --build "${_bin}"
                COMMENT "[mpapp] build native host child" USES_TERMINAL VERBATIM)
        endif()
        add_dependencies(mpapp-build-host mpapp-config-host)
        list(APPEND _build_all_deps mpapp-build-host)
    endif()

    # ---- cross children: portable core + tests per Zig triple ----------------
    foreach(p IN LISTS MPAPP_PLATFORMS)
        set(_bin "${CMAKE_BINARY_DIR}/${p}")
        _mpapp_child_config_target(mpapp-config-${p} "${_bin}" "${_tc}/${p}.cmake" "${p} (Zig)")
        add_custom_target(mpapp-build-${p}
            COMMAND ${CMAKE_COMMAND} --build "${_bin}"
            COMMENT "[mpapp] build ${p}" USES_TERMINAL VERBATIM)
        add_dependencies(mpapp-build-${p} mpapp-config-${p})
        list(APPEND _build_all_deps mpapp-build-${p})
    endforeach()

    add_custom_target(mpapp-all ALL DEPENDS ${_build_all_deps}
        COMMENT "[mpapp] build every host-supported platform")
endfunction()

# Internal: emit a `cmake -S . -B <bin>` configure target for a single-config
# (Ninja / Makefiles) child. toolchain may be "" for the native host child.
function(_mpapp_child_config_target name bin toolchain comment)
    set(_cmd ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}" -B "${bin}")
    if(MPAPP_NINJA)
        list(APPEND _cmd -G Ninja)
    endif()
    if(NOT toolchain STREQUAL "")
        # Cross child: pin the Zig toolchain and skip tests — Catch2's
        # discovery runs the test exe, which a cross binary can't do on the
        # host. The child still compiles mpapp-core, the real portability proof.
        list(APPEND _cmd "-DCMAKE_TOOLCHAIN_FILE=${toolchain}" -DBUILD_TESTING=OFF)
    endif()
    list(APPEND _cmd -DMPAPP_SUPERBUILD=OFF -DCMAKE_BUILD_TYPE=Debug)
    add_custom_target(${name}
        COMMAND ${_cmd}
        COMMENT "[mpapp] configure ${comment}"
        USES_TERMINAL VERBATIM)
endfunction()
