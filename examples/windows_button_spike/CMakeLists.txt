# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0003 — WinUI 3 button spike.
#
# Builds the unpackaged WinUI 3 desktop example. The Windows App SDK and
# C++/WinRT are provisioned via NuGet at configure time. WinUI 3 runtime
# DLLs are copied next to the resulting EXE so the unpackaged bootstrap
# can locate them at startup.

include(WindowsAppSDK)
mpapp_install_windows_app_sdk()

# Build the MPAPP Windows handler translation units into a small static
# library so the example links against them just like a real consumer
# would. (mpapp-core is platform-neutral; handlers live in their own TU.)
add_library(mpapp-handlers-windows STATIC
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/button_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/label_handler.cpp
)

target_include_directories(mpapp-handlers-windows PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(mpapp-handlers-windows PUBLIC cxx_std_23)
target_compile_definitions(mpapp-handlers-windows PUBLIC
    WINRT_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

# Generate cppwinrt projection and propagate its include dir. The helper
# sets it as PUBLIC on the target so consumers (the example exe) pick
# it up automatically.
mpapp_generate_winrt_projection(mpapp-handlers-windows MPAPP_WINRT_PROJECTION_DIR)

# /W4 /WX from the root CMakeLists is too strict for WinUI 3 headers in
# v2.0.1 (they emit a few deprecation warnings). Loosen to /W3 here.
target_compile_options(mpapp-handlers-windows PRIVATE
    /W3
    /Zc:__cplusplus
)

target_link_libraries(mpapp-handlers-windows PUBLIC
    mpapp-core
)

# Now the executable.
set(_winappsdk_inc "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Foundation/include")

add_executable(windows_button_spike WIN32
    main.cpp
    app.manifest
)

target_include_directories(windows_button_spike PRIVATE
    "${_winappsdk_inc}"
    "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Runtime/include"
)

target_link_libraries(windows_button_spike PRIVATE
    mpapp-core
    mpapp-handlers-windows
)

target_compile_features(windows_button_spike PRIVATE cxx_std_23)
target_compile_definitions(windows_button_spike PRIVATE
    WINRT_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

target_compile_options(windows_button_spike PRIVATE
    /W3
    /Zc:__cplusplus
)

mpapp_add_winappsdk_runtime(windows_button_spike)

# wWinMain entry point — the linker default for /SUBSYSTEM:WINDOWS with a
# wWinMain entry point.
set_target_properties(windows_button_spike PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS "/ENTRY:wWinMainCRTStartup"
)
