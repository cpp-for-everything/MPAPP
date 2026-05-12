# cmake/toolchains/linux-arm64.cmake
#
# Cross-compile MPAPP for Linux aarch64 using Zig.
# Per ADR-0011-cross-compilation-toolchain.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(MPAPP_ZIG_TARGET "aarch64-linux-gnu")

include("${CMAKE_CURRENT_LIST_DIR}/zig-common.cmake")
