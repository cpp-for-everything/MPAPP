# cmake/toolchains/linux-arm64.cmake
#
# Cross-compile to Linux aarch64 (glibc) via Zig.
# See cmake/toolchains/zig.cmake for the helper definition.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

include("${CMAKE_CURRENT_LIST_DIR}/zig.cmake")
mpapp_use_zig_target(aarch64-linux-gnu)
