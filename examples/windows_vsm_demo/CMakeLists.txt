# SPDX-License-Identifier: Apache-2.0
# RFC-0006 Visual State Manager demo — WinUI 3 / Windows.
#
# Reuses the mpapp-handlers-windows static library declared by the
# sibling examples/windows_button_spike target (the root
# examples/CMakeLists adds it first, then every Windows-side example
# links against it).
#
# Each Windows demo independently runs mpapp_install_windows_app_sdk()
# so the WINAPPSDK variables land in this directory's scope — the
# PARENT_SCOPE export from windows_button_spike's call does not
# propagate sideways into sibling subdirectories.

include(WindowsAppSDK)
mpapp_install_windows_app_sdk()

set(_winappsdk_inc "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Foundation/include")

add_executable(windows_vsm_demo
    main.cpp
    app.manifest
)

target_include_directories(windows_vsm_demo PRIVATE
    "${_winappsdk_inc}"
    "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Runtime/include"
)

target_link_libraries(windows_vsm_demo PRIVATE
    mpapp-core
    mpapp-handlers-windows
)

target_compile_features(windows_vsm_demo PRIVATE cxx_std_23)
target_compile_definitions(windows_vsm_demo PRIVATE
    WINRT_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

target_compile_options(windows_vsm_demo PRIVATE
    /W3
    /Zc:__cplusplus
)

mpapp_add_winappsdk_runtime(windows_vsm_demo)
