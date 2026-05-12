# cmake/toolchains/linux-x64.cmake
#
# Cross-compile to Linux x86_64 (glibc) via Zig.
# See cmake/toolchains/zig.cmake for the helper definition.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

include("${CMAKE_CURRENT_LIST_DIR}/zig.cmake")
mpapp_use_zig_target(x86_64-linux-gnu)
