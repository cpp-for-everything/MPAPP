# cmake/toolchains/zig.cmake
#
# Common helper for the Zig-based cross-compilation toolchain files.
# Per ADR-0011, Zig (zig cc / zig c++) is MPAPP's primary cross-compilation
# toolchain. This file:
#
#   * Pins the Zig version (single source of truth for the project).
#   * Locates the `zig` executable, preferring the auto-install location
#     written by the `mpapp` CLI and falling back to PATH.
#   * Provides `mpapp_use_zig_target(<triple>)` which generates a tiny
#     wrapper script per build directory and wires it into
#     CMAKE_C_COMPILER / CMAKE_CXX_COMPILER so the rest of CMake's
#     cross-compilation machinery "just works".
#
# This file is included by each per-platform toolchain file
# (windows-x64.cmake, linux-x64.cmake, etc.). It is not invoked
# directly by users.
#
# See also:
#   * vault/20_ADRs/ADR-0011-cross-compilation-toolchain.md
#   * vault/10_Architecture/Build System.md
#   * cmake/toolchains/README.md

# -- Re-entrancy guard ---------------------------------------------------------
#
# CMake re-runs the toolchain file multiple times during a configure (once
# during the initial scratch pass and again during the real pass). We define
# the helper function unconditionally but guard side-effecting work so we
# don't re-run `find_program` or emit duplicate diagnostics.
if(DEFINED MPAPP_ZIG_TOOLCHAIN_LOADED)
    return()
endif()
set(MPAPP_ZIG_TOOLCHAIN_LOADED TRUE)

# -- Pinned Zig version --------------------------------------------------------
#
# Update this single value to roll Zig across the entire project. The CI
# workflows (T-0006) and the `mpapp` CLI's auto-install logic key off this
# same string.
set(MPAPP_ZIG_VERSION "0.13.0" CACHE STRING "Pinned Zig version for MPAPP cross-compilation")

# -- Default auto-install location --------------------------------------------
#
# The `mpapp` CLI downloads Zig to a per-user cache. Layout:
#   Windows: %USERPROFILE%\.mpapp\toolchains\zig-<version>\zig.exe
#   POSIX:   $HOME/.mpapp/toolchains/zig-<version>/zig
#
# We compute the default here; users can override by setting MPAPP_ZIG_HOME
# in their environment or as a CMake cache variable before configuring.
if(NOT DEFINED MPAPP_ZIG_HOME)
    if(DEFINED ENV{MPAPP_ZIG_HOME})
        set(MPAPP_ZIG_HOME "$ENV{MPAPP_ZIG_HOME}")
    elseif(CMAKE_HOST_WIN32 OR WIN32)
        set(MPAPP_ZIG_HOME "$ENV{USERPROFILE}/.mpapp/toolchains/zig-${MPAPP_ZIG_VERSION}")
    else()
        set(MPAPP_ZIG_HOME "$ENV{HOME}/.mpapp/toolchains/zig-${MPAPP_ZIG_VERSION}")
    endif()
endif()

# -- Locate zig ----------------------------------------------------------------
#
# Prefer the auto-install location (HINTS searched first), fall back to PATH.
find_program(ZIG_EXECUTABLE
    NAMES zig zig.exe
    HINTS "${MPAPP_ZIG_HOME}"
    DOC "Path to the Zig executable used as a C/C++ cross-compiler driver"
)

if(NOT ZIG_EXECUTABLE)
    message(FATAL_ERROR
        "MPAPP: could not find a `zig` executable.\n"
        "Install Zig ${MPAPP_ZIG_VERSION} to ${MPAPP_ZIG_HOME} or add it to PATH.\n"
        "The `mpapp` CLI will auto-install on first cross-compile; until then, "
        "download from https://ziglang.org/download/ and unpack to the path above.\n"
        "Override the search path by setting MPAPP_ZIG_HOME in the environment "
        "or passing -DMPAPP_ZIG_HOME=<dir> to cmake."
    )
endif()

# -- Helper: emit per-build-dir wrapper scripts and wire them in --------------
#
# CMake's compiler-detection step runs the compiler with no easy way to
# inject extra arguments (CMAKE_C_COMPILER_ARG1 exists but is brittle when
# combined with launcher tooling like ccache). The robust pattern, used by
# the Zig project's own CMake samples, is to `configure_file` a tiny script
# that forwards everything to `zig cc --target=<triple>` and point
# CMAKE_C_COMPILER at the generated script.
#
# Reference: https://github.com/ziglang/zig/wiki/Building-Zig-using-CMake
function(mpapp_use_zig_target triple)
    set(MPAPP_ZIG_TRIPLE "${triple}" CACHE STRING "Active Zig target triple" FORCE)

    # CMAKE_BINARY_DIR is not yet set during early toolchain processing; fall
    # back to a sibling of MPAPP_ZIG_HOME so wrappers persist across builds.
    if(DEFINED CMAKE_BINARY_DIR AND NOT CMAKE_BINARY_DIR STREQUAL "")
        set(_wrapper_dir "${CMAKE_BINARY_DIR}/mpapp-zig-wrappers")
    else()
        set(_wrapper_dir "${MPAPP_ZIG_HOME}/wrappers/${triple}")
    endif()
    file(MAKE_DIRECTORY "${_wrapper_dir}")

    if(CMAKE_HOST_WIN32 OR WIN32)
        # -- Windows: emit .cmd shims ------------------------------------------
        set(_ext ".cmd")
        set(_cc_path     "${_wrapper_dir}/zig-cc${_ext}")
        set(_cxx_path    "${_wrapper_dir}/zig-c++${_ext}")
        set(_ar_path     "${_wrapper_dir}/zig-ar${_ext}")
        set(_ranlib_path "${_wrapper_dir}/zig-ranlib${_ext}")
        file(WRITE "${_cc_path}"     "@echo off\r\n\"${ZIG_EXECUTABLE}\" cc --target=${triple} %*\r\n")
        file(WRITE "${_cxx_path}"    "@echo off\r\n\"${ZIG_EXECUTABLE}\" c++ --target=${triple} %*\r\n")
        file(WRITE "${_ar_path}"     "@echo off\r\n\"${ZIG_EXECUTABLE}\" ar %*\r\n")
        file(WRITE "${_ranlib_path}" "@echo off\r\n\"${ZIG_EXECUTABLE}\" ranlib %*\r\n")
    else()
        # -- POSIX: emit shell shims -------------------------------------------
        set(_cc_path     "${_wrapper_dir}/zig-cc")
        set(_cxx_path    "${_wrapper_dir}/zig-c++")
        set(_ar_path     "${_wrapper_dir}/zig-ar")
        set(_ranlib_path "${_wrapper_dir}/zig-ranlib")
        file(WRITE "${_cc_path}"     "#!/bin/sh\nexec \"${ZIG_EXECUTABLE}\" cc --target=${triple} \"$@\"\n")
        file(WRITE "${_cxx_path}"    "#!/bin/sh\nexec \"${ZIG_EXECUTABLE}\" c++ --target=${triple} \"$@\"\n")
        file(WRITE "${_ar_path}"     "#!/bin/sh\nexec \"${ZIG_EXECUTABLE}\" ar \"$@\"\n")
        file(WRITE "${_ranlib_path}" "#!/bin/sh\nexec \"${ZIG_EXECUTABLE}\" ranlib \"$@\"\n")
        # chmod +x — file(CHMOD) is CMake 3.19+; project baseline is 3.28+.
        set(_exec_perms OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        file(CHMOD "${_cc_path}"     PERMISSIONS ${_exec_perms})
        file(CHMOD "${_cxx_path}"    PERMISSIONS ${_exec_perms})
        file(CHMOD "${_ar_path}"     PERMISSIONS ${_exec_perms})
        file(CHMOD "${_ranlib_path}" PERMISSIONS ${_exec_perms})
    endif()

    # Promote to the parent scope (toolchain-file scope) so CMake's
    # compiler-detection step picks these up.
    set(CMAKE_C_COMPILER   "${_cc_path}"  PARENT_SCOPE)
    set(CMAKE_CXX_COMPILER "${_cxx_path}" PARENT_SCOPE)
    set(CMAKE_C_COMPILER_ID   "Clang" PARENT_SCOPE)
    set(CMAKE_CXX_COMPILER_ID "Clang" PARENT_SCOPE)

    # `ar` and `ranlib` are subcommands of `zig`, not standalone binaries —
    # point CMake at the dedicated wrappers so static-archive creation stays
    # within the Zig-provided toolchain rather than falling back to host
    # binutils (which may not understand the target object format).
    set(CMAKE_AR     "${_ar_path}"     CACHE FILEPATH "Zig archiver wrapper"  FORCE)
    set(CMAKE_RANLIB "${_ranlib_path}" CACHE FILEPATH "Zig ranlib wrapper"    FORCE)
endfunction()
