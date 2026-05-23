# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0019 — CollectionView item_template demo (Windows/WinUI 3).

include(WindowsAppSDK)
mpapp_install_windows_app_sdk()

set(_winappsdk_inc "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Foundation/include")

add_executable(windows_item_template_demo
    main.cpp
    "${CMAKE_SOURCE_DIR}/examples/windows_button_spike/app.manifest"
)

target_include_directories(windows_item_template_demo PRIVATE
    "${_winappsdk_inc}"
    "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Runtime/include"
)

target_link_libraries(windows_item_template_demo PRIVATE
    mpapp-core
    mpapp-handlers-windows
)

target_compile_features(windows_item_template_demo PRIVATE cxx_std_23)
target_compile_definitions(windows_item_template_demo PRIVATE
    WINRT_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

target_compile_options(windows_item_template_demo PRIVATE
    /W3
    /Zc:__cplusplus
)

mpapp_add_winappsdk_runtime(windows_item_template_demo)
