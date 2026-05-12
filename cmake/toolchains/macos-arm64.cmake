# cmake/toolchains/macos-arm64.cmake
#
# Cross-compile to macOS aarch64 (Apple Silicon) via Zig.
#
# IMPORTANT: binaries produced from non-macOS hosts are UNSIGNED. They are
# useful for compile-time sanity checking, but cannot be distributed or run
# on user machines without a signing pass on macOS (codesign + notarization).
# See ADR-0005 and vault/10_Architecture/Build System.md for the full
# signing story.
#
# See cmake/toolchains/zig.cmake for the helper definition.
set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR arm64)

include("${CMAKE_CURRENT_LIST_DIR}/zig.cmake")
mpapp_use_zig_target(aarch64-macos-none)

if(NOT CMAKE_HOST_APPLE)
    message(WARNING
        "MPAPP: building Apple targets from a non-macOS host produces UNSIGNED "
        "binaries. They will not run on macOS without a separate signing pass "
        "performed on a Mac. See vault/10_Architecture/Build System.md and "
        "ADR-0005-ios-macos-separate-interop for details."
    )
endif()
