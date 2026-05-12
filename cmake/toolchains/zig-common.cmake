# cmake/toolchains/zig-common.cmake
#
# Shared bootstrap for every Zig-based toolchain file. Locates the pinned Zig
# binary and emits small wrapper scripts that turn `zig cc`, `zig c++`,
# `zig ar`, and `zig ranlib` into single-token executables. CMake serialises
# its compiler/AR cache entries with ';' as the list separator and re-parses
# them on every reconfigure, which breaks multi-token settings like
#   set(CMAKE_AR "${ZIG_EXECUTABLE}" ar)
# Wrappers side-step that entirely.
#
# Callers set MPAPP_ZIG_TARGET (e.g. "x86_64-linux-gnu") before including
# this file, then this file populates CMAKE_C_COMPILER / CMAKE_CXX_COMPILER /
# CMAKE_AR / CMAKE_RANLIB.

if(DEFINED ENV{ZIG})
    set(_mpapp_zig "$ENV{ZIG}")
elseif(WIN32 AND EXISTS "$ENV{USERPROFILE}/.mpapp/toolchains/zig-0.13.0/zig.exe")
    set(_mpapp_zig "$ENV{USERPROFILE}/.mpapp/toolchains/zig-0.13.0/zig.exe")
elseif(EXISTS "$ENV{HOME}/.mpapp/toolchains/zig-0.13.0/zig")
    set(_mpapp_zig "$ENV{HOME}/.mpapp/toolchains/zig-0.13.0/zig")
else()
    find_program(_mpapp_zig zig REQUIRED)
endif()

file(TO_CMAKE_PATH "${_mpapp_zig}" _mpapp_zig)

if(NOT DEFINED MPAPP_ZIG_TARGET)
    message(FATAL_ERROR "zig-common.cmake: caller must set MPAPP_ZIG_TARGET first")
endif()

# Wrappers go in a stable per-target temp folder. CMAKE_BINARY_DIR is not
# defined yet during toolchain processing, so we cannot drop them in the
# build tree; keying off the target triple keeps parallel builds isolated.
if(WIN32)
    set(_mpapp_wrap_dir "$ENV{TEMP}/mpapp-zig-${MPAPP_ZIG_TARGET}")
else()
    set(_mpapp_wrap_dir "/tmp/mpapp-zig-${MPAPP_ZIG_TARGET}")
endif()
file(MAKE_DIRECTORY "${_mpapp_wrap_dir}")

if(WIN32)
    set(_ext ".cmd")
    set(_shebang "@echo off")
    set(_arg_glob "%*")
else()
    set(_ext "")
    set(_shebang "#!/bin/sh")
    set(_arg_glob "\"$@\"")
endif()

function(_mpapp_write_wrapper name)
    set(out "${_mpapp_wrap_dir}/${name}${_ext}")
    string(JOIN " " args_joined ${ARGN})
    file(WRITE "${out}" "${_shebang}\n\"${_mpapp_zig}\" ${args_joined} ${_arg_glob}\n")
    if(NOT WIN32)
        execute_process(COMMAND chmod +x "${out}")
    endif()
    set(${name}_PATH "${out}" PARENT_SCOPE)
endfunction()

_mpapp_write_wrapper(zig-cc  cc  -target ${MPAPP_ZIG_TARGET})
_mpapp_write_wrapper(zig-cxx c++ -target ${MPAPP_ZIG_TARGET})
_mpapp_write_wrapper(zig-ar  ar)
_mpapp_write_wrapper(zig-ranlib ranlib)

set(CMAKE_C_COMPILER   "${zig-cc_PATH}")
set(CMAKE_CXX_COMPILER "${zig-cxx_PATH}")
set(CMAKE_AR           "${zig-ar_PATH}")
set(CMAKE_RANLIB       "${zig-ranlib_PATH}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# CMake 4 enables clang-scan-deps for C++23 by default. Zig 0.13 does not
# bundle clang-scan-deps so we disable module scanning for cross builds.
# MPAPP does not use C++ named modules in its public surface yet.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
