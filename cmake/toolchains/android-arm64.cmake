# cmake/toolchains/android-arm64.cmake
#
# Cross-compile to Android aarch64 (API 24+) via Zig. Zig bundles a Bionic
# libc compatible with the Android NDK headers, so no separate NDK install
# is required for compilation. Final APK packaging still depends on the
# Android SDK build-tools — that lives in the `mpapp package` pipeline.
#
# See cmake/toolchains/zig.cmake for the helper definition.
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Minimum Android API level supported by MPAPP.
# (Android 7.0 / Nougat — the earliest release with reliable Vulkan and a
# C++17-capable Bionic. Newer features are runtime-gated.)
set(CMAKE_ANDROID_API 24)

include("${CMAKE_CURRENT_LIST_DIR}/zig.cmake")

# Zig accepts the Android API level as a triple suffix. This pins the
# minimum supported runtime libc surface to API 24 across the codebase.
mpapp_use_zig_target(aarch64-linux-android.24)
