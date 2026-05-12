# cmake/toolchains/ios-arm64.cmake
#
# Cross-compile MPAPP for iOS arm64 using Zig.
# Per ADR-0011-cross-compilation-toolchain. iOS binaries cross-built from
# non-Mac hosts are *unsigned* — they will not run on device/simulator until
# re-signed on macOS.

# CMake's iOS platform module hard-requires a Mac SDK on disk. Cross-building
# from Windows we don't have one, so we masquerade as Darwin and let Zig's
# -target argument drive the iOS-vs-macOS divergence. MPAPP_TARGET_IOS is set
# so downstream CMake logic can branch on iOS independently of the system
# name.
set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR arm64)
set(CMAKE_OSX_ARCHITECTURES arm64)
set(MPAPP_ZIG_TARGET "aarch64-ios-none")
set(MPAPP_TARGET_IOS ON CACHE BOOL "Building for iOS" FORCE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

include("${CMAKE_CURRENT_LIST_DIR}/zig-common.cmake")
