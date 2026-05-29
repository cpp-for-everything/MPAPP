# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. УИСС — cross-platform "Е-Студент" reference app.
#
# One source tree (main.cpp + uiss/*.hpp) builds against the platform
# handler set: GTK4 on Linux, WinUI 3 on Windows. Android builds the same
# sources through its own Gradle/NDK externalNativeBuild (see the
# android_uiss scaffolding). Cyrillic string literals require UTF-8 source
# interpretation — enforced per-compiler below.

# ---- Linux / GTK4 ------------------------------------------------------
if(UNIX AND NOT APPLE)
    add_executable(uiss main.cpp)
    target_include_directories(uiss PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(uiss PRIVATE
        mpapp-core
        mpapp-handlers-linux
    )
    target_compile_features(uiss PRIVATE cxx_std_23)
    target_compile_options(uiss PRIVATE
        -Wall -Wextra -Wpedantic
        -finput-charset=UTF-8
        -fexec-charset=UTF-8
    )
endif()

# ---- Windows / WinUI 3 -------------------------------------------------
if(WIN32)
    include(WindowsAppSDK)
    mpapp_install_windows_app_sdk()

    set(_winappsdk_inc "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Foundation/include")

    add_executable(uiss
        main.cpp
        "${CMAKE_SOURCE_DIR}/examples/windows_button_spike/app.manifest"
    )

    target_include_directories(uiss PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${_winappsdk_inc}"
        "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Runtime/include"
    )

    target_link_libraries(uiss PRIVATE
        mpapp-core
        mpapp-handlers-windows
    )

    target_compile_features(uiss PRIVATE cxx_std_23)
    target_compile_definitions(uiss PRIVATE
        WINRT_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
    )
    target_compile_options(uiss PRIVATE
        /W3
        /Zc:__cplusplus
        /utf-8
    )

    mpapp_add_winappsdk_runtime(uiss)
endif()
