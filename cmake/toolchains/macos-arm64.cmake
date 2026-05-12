# cmake/toolchains/macos-arm64.cmake
#
# Cross-compile MPAPP for macOS arm64 using Zig.
# Per ADR-0011-cross-compilation-toolchain. Apple targets cross-built from
# non-Mac hosts produce *unsigned* binaries — they will not run on Apple
# hardware until re-signed on macOS.

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR arm64)
set(CMAKE_OSX_ARCHITECTURES arm64)
set(MPAPP_ZIG_TARGET "aarch64-macos-none")

include("${CMAKE_CURRENT_LIST_DIR}/zig-common.cmake")
