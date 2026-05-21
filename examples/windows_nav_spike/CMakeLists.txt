# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. NavigationPage spike — drives ADR-0014's page_stack
# engine + ADR-0019's push_async/pop_async wrappers on Win32 + WinUI 3.

# `mpapp_install_windows_app_sdk` sets `MPAPP_WINAPPSDK_FOUNDATION_DIR` in
# the current scope; we need it before calling `mpapp_add_winappsdk_runtime`.
# (The button spike already installed the packages — the nuget call below
# short-circuits because the package dir exists.)
include(WindowsAppSDK)
mpapp_install_windows_app_sdk()

# The button spike already builds mpapp-handlers-windows from src/handlers/
# windows/*.cpp via glob; we reuse that target instead of rebuilding the
# handler library here.

set(_winappsdk_inc "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Foundation/include")

add_executable(windows_nav_spike
    main.cpp
    "${CMAKE_SOURCE_DIR}/examples/windows_button_spike/app.manifest"
)

target_include_directories(windows_nav_spike PRIVATE
    "${_winappsdk_inc}"
    "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Runtime/include"
)

target_link_libraries(windows_nav_spike PRIVATE
    mpapp-core
    mpapp-handlers-windows
)

target_compile_features(windows_nav_spike PRIVATE cxx_std_23)
target_compile_definitions(windows_nav_spike PRIVATE
    WINRT_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

target_compile_options(windows_nav_spike PRIVATE
    /W3
    /Zc:__cplusplus
)

mpapp_add_winappsdk_runtime(windows_nav_spike)
