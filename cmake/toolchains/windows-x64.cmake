# cmake/toolchains/windows-x64.cmake
#
# Cross-compile to Windows x86_64 (GNU ABI) via Zig.
# See cmake/toolchains/zig.cmake for the helper definition.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

include("${CMAKE_CURRENT_LIST_DIR}/zig.cmake")
mpapp_use_zig_target(x86_64-windows-gnu)
