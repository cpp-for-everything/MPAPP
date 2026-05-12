# cmake/toolchains/ios-arm64.cmake
#
# Cross-compile to iOS aarch64 (device) via Zig.
#
# IMPORTANT: binaries produced from non-macOS hosts are UNSIGNED. iOS in
# particular cannot run unsigned binaries on a real device or the
# Simulator without a signing + provisioning-profile pass on macOS.
# Use this toolchain for compile-time sanity checks; ship via macOS CI.
# See ADR-0005 and vault/10_Architecture/Build System.md for details.
#
# See cmake/toolchains/zig.cmake for the helper definition.
set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_SYSTEM_PROCESSOR arm64)

include("${CMAKE_CURRENT_LIST_DIR}/zig.cmake")
mpapp_use_zig_target(aarch64-ios-none)

if(NOT CMAKE_HOST_APPLE)
    message(WARNING
        "MPAPP: building Apple targets from a non-macOS host produces UNSIGNED "
        "binaries. iOS in particular requires a Mac for codesign + a valid "
        "provisioning profile before the artifact can run on device or "
        "Simulator. See vault/10_Architecture/Build System.md and "
        "ADR-0005-ios-macos-separate-interop for details."
    )
endif()
