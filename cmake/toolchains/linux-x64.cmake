# cmake/toolchains/linux-x64.cmake
#
# Cross-compile MPAPP for Linux x86_64 using Zig (zig cc / zig c++).
# Per ADR-0011-cross-compilation-toolchain.
#
# Usage:
#   cmake -S . -B build-linux-x64 \
#         -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x64.cmake \
#         -DMPAPP_BUILD_TOOLS=OFF -DMPAPP_BUILD_EXAMPLES=OFF -G Ninja

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(MPAPP_ZIG_TARGET "x86_64-linux-gnu")

include("${CMAKE_CURRENT_LIST_DIR}/zig-common.cmake")
