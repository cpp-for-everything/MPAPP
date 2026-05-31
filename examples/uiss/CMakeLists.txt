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

    # NOTE: the App.xaml WinUI-shell path below is WORK IN PROGRESS (T-0067).
    # It builds through midlrt (App.idl's `runtimeclass` parses) but is blocked
    # on the CppWinRT winmd output-dir coordination (MIDL2212). It is guarded to
    # the Visual Studio generator and is INERT under Ninja, so the default
    # toolchain + CI (Ninja) are unaffected. Opt in only to continue this work.
    if(CMAKE_GENERATOR MATCHES "Visual Studio" AND MPAPP_UISS_WINUI_SHELL_WIP)
        # ---- App.xaml-backed WinUI 3 shell (WIP — see T-0067) --------------
        # Unpackaged WinUI 3 needs the VS-C++-WinUI-template structure for
        # framework theme resources to resolve: an App.xaml (x:Class) whose
        # <XamlControlsResources/> triggers the MrtCore framework-resource merge
        # into resources.pri AND wires ms-appx:/// resolution, plus the XAML
        # compiler's generated entry point + IXamlMetadataProvider. The hand-
        # rolled code-only shell (Ninja path below) builds + runs but can't
        # resolve themeresources, so a TextBox crashes on first render. Requires
        # the Visual Studio generator (Ninja can't run the XAML compiler).
        add_executable(uiss WIN32
            winui/App.idl
            winui/App.xaml
            winui/App.xaml.cpp
            "${CMAKE_SOURCE_DIR}/examples/windows_button_spike/app.manifest"
        )
        set_source_files_properties(winui/App.xaml PROPERTIES
            VS_XAML_TYPE "ApplicationDefinition")
        set_target_properties(uiss PROPERTIES
            VS_GLOBAL_UseWinUI            "true"
            VS_GLOBAL_EnableMsixTooling   "true"
            VS_GLOBAL_WindowsPackageType  "None"
            VS_GLOBAL_AppxPackage         "false"
            VS_GLOBAL_WindowsAppSDKSelfContained "false"
            VS_GLOBAL_RootNamespace       "uiss")
        # CppWinRT writes the per-idl winmd to $(IntDir)Unmerged\ and the
        # projection under "$(IntDir)Generated Files\"; CMake doesn't create
        # those subdirs before midlrt runs (MIDL2212 path-not-found). A
        # PreBuild step (MSBuild expands $(IntDir)) makes them exist first.
        add_custom_command(TARGET uiss PRE_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory
                "$(IntDir)Unmerged"
                "$(IntDir)Merged"
                "$(IntDir)Generated Files")
        # CppWinRT's targets override the Midl build step to use midlrt (MIDL
        # 3.0 / WinRT), which the `runtimeclass App` syntax in App.idl needs;
        # classic midl rejects `namespace`.
        set_property(TARGET uiss PROPERTY
            VS_PACKAGE_REFERENCES "Microsoft.WindowsAppSDK_1.8.260416003;Microsoft.Windows.CppWinRT_2.0.250303.1")
    else()
        # ---- Ninja fallback: builds + links + runs, but framework theme
        #      resources don't resolve (T-0067) — kept so the Ninja toolchain
        #      stays green for everything else.
        add_executable(uiss
            main.cpp
            "${CMAKE_SOURCE_DIR}/examples/windows_button_spike/app.manifest"
        )
    endif()

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
        /Zc:__cplusplus
        /utf-8
    )

    mpapp_add_winappsdk_runtime(uiss)
endif()

# Bake the source-tree asset dir in so the app can load the bundled TU logo
# by absolute path (demonstrates the real per-platform image loaders).
if(TARGET uiss)
    target_compile_definitions(uiss PRIVATE
        UISS_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
endif()
