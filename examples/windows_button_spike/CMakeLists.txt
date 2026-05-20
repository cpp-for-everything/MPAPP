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
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/application_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/window_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/stack_layout_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/entry_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/switch_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/check_box_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/radio_button_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/slider_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/stepper_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/editor_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/scroll_view_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/box_view_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/border_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/activity_indicator_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/progress_bar_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/search_bar_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/picker_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/windows/date_picker_handler.cpp
)

target_include_directories(mpapp-handlers-windows PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# application_handler.cpp pulls in <MddBootstrap.h> + the WindowsAppRuntime
# extern declaration. The headers live in the Foundation NuGet's include
# dir; add it SYSTEM so /W4 /WX in upstream targets does not flag SDK
# headers.
target_include_directories(mpapp-handlers-windows SYSTEM PRIVATE
    "${MPAPP_WINAPPSDK_FOUNDATION_DIR}/include"
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

add_executable(windows_button_spike
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

# T-0011: this is now a console-subsystem app so the cross-platform
# `int main(int, char**)` entry works without a wWinMain shim. A
# console window appears alongside the WinUI window for the spike —
# acceptable for development. A `/SUBSYSTEM:WINDOWS` flavour with the
# console hidden via `FreeConsole()` lands when the spike is promoted
# to a real sample app.
