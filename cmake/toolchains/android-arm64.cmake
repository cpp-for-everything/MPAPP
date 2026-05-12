# cmake/toolchains/android-arm64.cmake
#
# Cross-compile MPAPP for Android aarch64 using Zig.
# Per ADR-0011-cross-compilation-toolchain.
#
# Zig bundles libc/headers for Android (bionic) so the NDK is not required for
# the basic compile/link path. For JNI integration the NDK headers are still
# needed but the smoke target only exercises the C++ pipeline.
#
# NOTE: We set CMAKE_SYSTEM_NAME to "Linux" instead of "Android" to bypass
# CMake's Android-Determine module which mandates a discoverable NDK. The
# Android target triple passed to Zig via -target aarch64-linux-android still
# selects bionic libc/ABI. MPAPP_TARGET_ANDROID is set so MPAPP_CMake logic
# can branch on the Android-ness independently.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
# Zig 0.13.0 does NOT ship a bionic libc for Android (verified empirically via
# `zig targets | jq .libc` — only musl/gnu Linux variants are bundled). That
# means object compilation works, archiving into a .a works, but linking a
# full executable does not until the NDK is on the include path.
# The smoke target builds mpapp-core as a static library, which only needs
# the compile step, so we constrain CMake's compiler-detection probe to
# static-library mode to side-step its default executable link test.
set(MPAPP_ZIG_TARGET "aarch64-linux-android")
set(MPAPP_TARGET_ANDROID ON CACHE BOOL "Building for Android" FORCE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

include("${CMAKE_CURRENT_LIST_DIR}/zig-common.cmake")
