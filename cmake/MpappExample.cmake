# cmake/MpappExample.cmake
#
# Thin helpers so each examples/<app>/CMakeLists.txt is a one-line recipe.
# The repetitive native-SDK wiring (WinUI runtime copy, handler linking,
# warning policy) lives here, not copy-pasted into ~30 leaf files.
#
#   mpapp_add_example(<name> SOURCES <...> [LINK <...>] [STRICT])
#       Plain cross-platform exe linking mpapp-core. RELAXED warnings by
#       default; pass STRICT for first-party-only demos (-Werror / /WX).
#
#   mpapp_add_linux_example(<name> [SOURCES <...>])
#       GTK4 exe linking mpapp-core + mpapp-handlers-linux. SOURCES defaults
#       to main.cpp.
#
#   mpapp_add_windows_example(<name> [SOURCES <...>] [DEFINES <...>] [OPTIONS <...>])
#       Unpackaged WinUI 3 exe linking mpapp-core + mpapp-handlers-windows,
#       with the shared app.manifest, WinAppSDK include dirs, and the runtime
#       DLL copy. SOURCES defaults to main.cpp.
include_guard(GLOBAL)

include(MpappWarnings)

# Shared unpackaged-WinUI-3 application manifest (lives with the button spike).
set(MPAPP_WIN_APP_MANIFEST
    "${CMAKE_SOURCE_DIR}/examples/windows_button_spike/app.manifest"
    CACHE INTERNAL "Shared WinUI 3 unpackaged app manifest")

function(mpapp_add_example name)
    cmake_parse_arguments(A "STRICT" "" "SOURCES;LINK;OPTIONS" ${ARGN})
    add_executable(${name} ${A_SOURCES})
    target_link_libraries(${name} PRIVATE mpapp-core ${A_LINK})
    target_compile_features(${name} PRIVATE cxx_std_23)
    if(A_STRICT)
        mpapp_apply_warnings(${name})
    else()
        mpapp_apply_warnings(${name} RELAXED)
    endif()
    if(A_OPTIONS)
        target_compile_options(${name} PRIVATE ${A_OPTIONS})
    endif()
endfunction()

function(mpapp_add_linux_example name)
    cmake_parse_arguments(A "" "" "SOURCES;OPTIONS" ${ARGN})
    if(NOT A_SOURCES)
        set(A_SOURCES main.cpp)
    endif()
    add_executable(${name} ${A_SOURCES})
    target_link_libraries(${name} PRIVATE mpapp-core mpapp-handlers-linux)
    target_compile_features(${name} PRIVATE cxx_std_23)
    target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic ${A_OPTIONS})
endfunction()

function(mpapp_add_windows_example name)
    cmake_parse_arguments(A "" "" "SOURCES;DEFINES;OPTIONS" ${ARGN})
    if(NOT A_SOURCES)
        set(A_SOURCES main.cpp)
    endif()

    include(WindowsAppSDK)
    mpapp_install_windows_app_sdk()

    add_executable(${name} ${A_SOURCES} "${MPAPP_WIN_APP_MANIFEST}")
    target_include_directories(${name} PRIVATE
        "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Foundation/include"
        "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Runtime/include")
    target_link_libraries(${name} PRIVATE mpapp-core mpapp-handlers-windows)
    target_compile_features(${name} PRIVATE cxx_std_23)
    target_compile_definitions(${name} PRIVATE
        WINRT_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE ${A_DEFINES})
    target_compile_options(${name} PRIVATE /W3 /Zc:__cplusplus ${A_OPTIONS})
    mpapp_add_winappsdk_runtime(${name})
endfunction()
