# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. Windows counterpart of gtk4_tableview_demo — builds a
# TableView whose rows are real cell-tree instances on WinUI 3.
#
# Mirrors examples/windows_nav_spike/CMakeLists.txt — relies on the
# button spike to have already configured mpapp-handlers-windows + the
# WinAppSDK NuGet packages.

include(WindowsAppSDK)
mpapp_install_windows_app_sdk()

set(_winappsdk_inc "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Foundation/include")

add_executable(windows_tableview_demo
    main.cpp
    "${CMAKE_SOURCE_DIR}/examples/windows_button_spike/app.manifest"
)

target_include_directories(windows_tableview_demo PRIVATE
    "${_winappsdk_inc}"
    "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Runtime/include"
)

target_link_libraries(windows_tableview_demo PRIVATE
    mpapp-core
    mpapp-handlers-windows
)

target_compile_features(windows_tableview_demo PRIVATE cxx_std_23)
target_compile_definitions(windows_tableview_demo PRIVATE
    WINRT_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
)

target_compile_options(windows_tableview_demo PRIVATE
    /W3
    /Zc:__cplusplus
)

mpapp_add_winappsdk_runtime(windows_tableview_demo)
