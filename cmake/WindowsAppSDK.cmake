# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0003 — WinUI 3 button spike.
#
# Provision the Windows App SDK (unpackaged WinUI 3) for consumption by
# CMake/Ninja targets. There is no `find_package(WindowsAppSDK)` shipped
# by Microsoft, so we install the NuGet packages locally and stitch
# together include / library / runtime dirs.
#
# After include(WindowsAppSDK):
#
#   * mpapp_install_windows_app_sdk() restores the NuGet packages.
#   * mpapp_generate_winrt_projection(<target> <out_dir>) runs cppwinrt
#     against the WinMD files and adds the generated headers to <target>.
#   * mpapp_add_winappsdk_runtime(<target>) configures includes, link
#     libraries, and post-build copy of the WinUI runtime DLLs.

set(MPAPP_NUGET_VERSION_CPPWINRT "2.0.250303.1")
# WindowsAppSDK 1.8.x — chosen because the corresponding 1.8 runtime
# framework package is preinstalled on Windows 11 24H2 development
# images (Get-AppxPackage shows Microsoft.WindowsAppRuntime.1.8). When
# bumping this, update `majorMinorVersion` in
# examples/windows_button_spike/main.cpp too.
set(MPAPP_NUGET_VERSION_WINAPPSDK "1.8.260416003")
set(MPAPP_NUGET_URL "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe")

set(MPAPP_PACKAGES_DIR "${CMAKE_BINARY_DIR}/packages" CACHE PATH
    "Directory where WindowsAppSDK NuGet packages are installed.")
set(MPAPP_NUGET_EXE "${CMAKE_BINARY_DIR}/nuget/nuget.exe" CACHE FILEPATH
    "Path to nuget.exe (downloaded if missing).")

function(_mpapp_download_nuget)
    if(EXISTS "${MPAPP_NUGET_EXE}")
        return()
    endif()
    get_filename_component(_nuget_dir "${MPAPP_NUGET_EXE}" DIRECTORY)
    file(MAKE_DIRECTORY "${_nuget_dir}")
    message(STATUS "MPAPP: downloading nuget.exe to ${MPAPP_NUGET_EXE}")
    file(DOWNLOAD "${MPAPP_NUGET_URL}" "${MPAPP_NUGET_EXE}"
         STATUS _dl_status SHOW_PROGRESS)
    list(GET _dl_status 0 _dl_code)
    if(NOT _dl_code EQUAL 0)
        message(FATAL_ERROR "MPAPP: failed to download nuget.exe — ${_dl_status}")
    endif()
endfunction()

function(_mpapp_nuget_install pkg version)
    if(EXISTS "${MPAPP_PACKAGES_DIR}/${pkg}")
        return()
    endif()
    _mpapp_download_nuget()
    message(STATUS "MPAPP: nuget install ${pkg} ${version}")
    execute_process(
        COMMAND "${MPAPP_NUGET_EXE}" install "${pkg}"
                -Version "${version}"
                -OutputDirectory "${MPAPP_PACKAGES_DIR}"
                -ExcludeVersion
                -Source "https://api.nuget.org/v3/index.json"
        RESULT_VARIABLE _nuget_rc
        OUTPUT_VARIABLE _nuget_out
        ERROR_VARIABLE  _nuget_err)
    if(NOT _nuget_rc EQUAL 0)
        message(FATAL_ERROR
            "MPAPP: nuget install failed for ${pkg} ${version}\n"
            "stdout: ${_nuget_out}\nstderr: ${_nuget_err}")
    endif()
endfunction()

function(mpapp_install_windows_app_sdk)
    _mpapp_nuget_install(Microsoft.Windows.CppWinRT "${MPAPP_NUGET_VERSION_CPPWINRT}")
    _mpapp_nuget_install(Microsoft.WindowsAppSDK    "${MPAPP_NUGET_VERSION_WINAPPSDK}")

    # Expose paths to the caller.
    set(MPAPP_CPPWINRT_EXE
        "${MPAPP_PACKAGES_DIR}/Microsoft.Windows.CppWinRT/bin/cppwinrt.exe"
        PARENT_SCOPE)
    set(MPAPP_CPPWINRT_FAST_FORWARDER_LIB
        "${MPAPP_PACKAGES_DIR}/Microsoft.Windows.CppWinRT/build/native/lib/x64/cppwinrt_fast_forwarder.lib"
        PARENT_SCOPE)
    set(MPAPP_WINAPPSDK_DIR
        "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK"
        PARENT_SCOPE)
    set(MPAPP_WINAPPSDK_FOUNDATION_DIR
        "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Foundation"
        PARENT_SCOPE)
    set(MPAPP_WINAPPSDK_WINUI_DIR
        "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.WinUI"
        PARENT_SCOPE)
    set(MPAPP_WINAPPSDK_INTERACTIVE_DIR
        "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.InteractiveExperiences"
        PARENT_SCOPE)
    set(MPAPP_WINAPPSDK_WIDGETS_DIR
        "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.Widgets"
        PARENT_SCOPE)
    set(MPAPP_WINAPPSDK_AI_DIR
        "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.AI"
        PARENT_SCOPE)
    # Renamed in WindowsAppSDK 1.8.260416003+ from
    # Microsoft.Windows.AI.MachineLearning to Microsoft.WindowsAppSDK.ML.
    # The old folder name lingers in some derived caches; prefer the new
    # one if present.
    if(EXISTS "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.ML/metadata")
        set(MPAPP_WINDOWS_AI_ML_DIR
            "${MPAPP_PACKAGES_DIR}/Microsoft.WindowsAppSDK.ML"
            PARENT_SCOPE)
    else()
        set(MPAPP_WINDOWS_AI_ML_DIR
            "${MPAPP_PACKAGES_DIR}/Microsoft.Windows.AI.MachineLearning"
            PARENT_SCOPE)
    endif()
    set(MPAPP_WEBVIEW2_DIR
        "${MPAPP_PACKAGES_DIR}/Microsoft.Web.WebView2"
        PARENT_SCOPE)
endfunction()

# Run cppwinrt to generate the projection headers for the WinUI 3 surface.
#
# Usage:
#   mpapp_generate_winrt_projection(<target> <out_var>)
#
# After the call, <out_var> contains the directory holding the generated
# headers; add it to your target's include path. cppwinrt outputs
# include/winrt/*.h that match `#include <winrt/Microsoft.UI.Xaml.h>` etc.
function(mpapp_generate_winrt_projection target out_var)
    set(_out "${CMAKE_BINARY_DIR}/winrt_projection")
    set(_stamp "${_out}/.cppwinrt.stamp")
    # cppwinrt writes headers directly into <out>/winrt/, so the include
    # path the compiler sees is <out>/ itself.
    set(_include "${_out}")
    set(_sentinel "${_out}/winrt/base.h")

    set(_foundation_md "${MPAPP_WINAPPSDK_FOUNDATION_DIR}/metadata")
    set(_winui_md      "${MPAPP_WINAPPSDK_WINUI_DIR}/metadata")
    set(_interactive_md "${MPAPP_WINAPPSDK_INTERACTIVE_DIR}/metadata/10.0.18362.0")
    set(_widgets_md    "${MPAPP_WINAPPSDK_WIDGETS_DIR}/metadata")
    set(_ai_md         "${MPAPP_WINAPPSDK_AI_DIR}/metadata")
    set(_aiml_md       "${MPAPP_WINDOWS_AI_ML_DIR}/metadata")
    set(_webview2_md   "${MPAPP_WEBVIEW2_DIR}/lib")

    # Generate at configure time on first run so dependency-scanning
    # (e.g. C++ modules ddi pass) sees the headers immediately. The
    # custom command keeps the headers in sync on subsequent builds.
    if(NOT EXISTS "${_sentinel}")
        message(STATUS "MPAPP: bootstrapping cppwinrt projection (one-time)")
        file(MAKE_DIRECTORY "${_out}")
        execute_process(
            COMMAND "${MPAPP_CPPWINRT_EXE}"
                    -in 10.0.26100.0
                    -in "${_foundation_md}"
                    -in "${_winui_md}"
                    -in "${_interactive_md}"
                    -in "${_widgets_md}"
                    -in "${_ai_md}"
                    -in "${_aiml_md}"
                    -in "${_webview2_md}"
                    -out "${_out}"
            RESULT_VARIABLE _cppwinrt_rc
            OUTPUT_VARIABLE _cppwinrt_out
            ERROR_VARIABLE  _cppwinrt_err)
        if(NOT _cppwinrt_rc EQUAL 0)
            message(FATAL_ERROR
                "MPAPP: cppwinrt failed (rc=${_cppwinrt_rc})\n"
                "stdout: ${_cppwinrt_out}\nstderr: ${_cppwinrt_err}")
        endif()
        file(TOUCH "${_stamp}")
    endif()

    add_custom_command(
        OUTPUT "${_stamp}"
        COMMAND "${MPAPP_CPPWINRT_EXE}"
                -in 10.0.26100.0
                -in "${_foundation_md}"
                -in "${_winui_md}"
                -in "${_interactive_md}"
                -in "${_widgets_md}"
                -in "${_ai_md}"
                -in "${_aiml_md}"
                -in "${_webview2_md}"
                -out "${_out}"
        COMMAND "${CMAKE_COMMAND}" -E touch "${_stamp}"
        COMMENT "MPAPP: regenerating C++/WinRT projection headers"
        VERBATIM)

    add_custom_target(${target}_cppwinrt DEPENDS "${_stamp}")
    add_dependencies(${target} ${target}_cppwinrt)
    target_include_directories(${target} SYSTEM PUBLIC "${_include}")

    set(${out_var} "${_out}" PARENT_SCOPE)
endfunction()

# Add Windows App SDK / WinUI 3 includes, link libs, and runtime DLL copy
# to <target>. Call after mpapp_generate_winrt_projection.
function(mpapp_add_winappsdk_runtime target)
    target_include_directories(${target} SYSTEM PRIVATE
        "${MPAPP_WINAPPSDK_FOUNDATION_DIR}/include"
    )

    target_link_libraries(${target} PRIVATE
        "${MPAPP_WINAPPSDK_FOUNDATION_DIR}/lib/native/x64/Microsoft.WindowsAppRuntime.Bootstrap.lib"
        "${MPAPP_WINAPPSDK_FOUNDATION_DIR}/lib/native/x64/Microsoft.WindowsAppRuntime.lib"
        "${MPAPP_CPPWINRT_FAST_FORWARDER_LIB}"
        windowsapp
    )

    # Runtime DLLs that must sit next to the EXE so the loader can
    # resolve imports BEFORE the bootstrap runs (the implicit import of
    # Microsoft.WindowsAppRuntime.dll cannot wait for MddBootstrapInitialize).
    # Bootstrap.dll is needed for MddBootstrap*. WindowsAppRuntime.dll
    # is the WinRT-type registrar; without it Application::Start fails
    # with RPC_E_WRONG_THREAD because the type system can't resolve the
    # WinUI activation context.
    set(_rt_foundation        "${MPAPP_WINAPPSDK_FOUNDATION_DIR}/runtimes/win-x64/native")
    set(_rt_foundation_fw     "${MPAPP_WINAPPSDK_FOUNDATION_DIR}/runtimes-framework/win-x64/native")
    # WebView2 runtime loader + WinRT projection. The WinUI 3 WebView2
    # control depends on these at first-use time (the implicit
    # EnsureCoreWebView2Async issued by Source assignment or our
    # async_init coroutine). Without them, init fails with
    # HRESULT 0x8007007E (ERROR_MOD_NOT_FOUND). They're harmless to
    # ship for targets that never instantiate a WebView2 — the OS only
    # loads them on first use.
    set(_rt_webview2_loader   "${MPAPP_WEBVIEW2_DIR}/runtimes/win-x64/native/WebView2Loader.dll")
    set(_rt_webview2_core     "${MPAPP_WEBVIEW2_DIR}/runtimes/win-x64/native_uap/Microsoft.Web.WebView2.Core.dll")
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${_rt_foundation}/Microsoft.WindowsAppRuntime.Bootstrap.dll"
                "$<TARGET_FILE_DIR:${target}>/Microsoft.WindowsAppRuntime.Bootstrap.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${_rt_foundation_fw}/Microsoft.WindowsAppRuntime.dll"
                "$<TARGET_FILE_DIR:${target}>/Microsoft.WindowsAppRuntime.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${_rt_webview2_loader}"
                "$<TARGET_FILE_DIR:${target}>/WebView2Loader.dll"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${_rt_webview2_core}"
                "$<TARGET_FILE_DIR:${target}>/Microsoft.Web.WebView2.Core.dll"
        COMMENT "MPAPP: copying WindowsAppRuntime + WebView2 DLLs next to ${target}"
        VERBATIM)

    # --- Unpackaged resources.pri (WinUI control theme resources) ----------
    # A code-only WinUI 3 exe (raw add_executable, no MSBuild project) emits no
    # resources.pri, so `ms-appx:///Microsoft.UI.Xaml/Themes/themeresources.xaml`
    # is unresolvable and XamlControlsResources fails at runtime (E_FAIL /
    # stowed XAML exception ~300ms after the first templated control). Fix:
    # merge the WinUI controls PRI into an app resources.pri whose primary
    # resource map is "Application" (omit makepri /IndexName — the runtime
    # infers the Application root for unpackaged apps). Built once, staged per
    # exe. The bootstrap half (IXamlMetadataProvider + XamlControlsResources)
    # lives in src/handlers/windows/application_handler.cpp.
    if(NOT MPAPP_MAKEPRI)
        find_program(MPAPP_MAKEPRI makepri PATHS
            "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64"
            "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/x64"
            "C:/Program Files (x86)/Windows Kits/10/bin/x64")
    endif()
    set(_pri_ctrl
        "${MPAPP_WINAPPSDK_WINUI_DIR}/runtimes-framework/win-x64/native/Microsoft.UI.Xaml.Controls.pri")
    if(MPAPP_MAKEPRI AND EXISTS "${_pri_ctrl}")
        set(_pri_dir  "${CMAKE_BINARY_DIR}/winui-pri")
        set(_pri_proj "${_pri_dir}/proj")
        set(_pri_out  "${_pri_dir}/resources.pri")
        if(NOT TARGET mpapp_winui_resources_pri)
            add_custom_command(
                OUTPUT "${_pri_out}"
                COMMAND "${CMAKE_COMMAND}" -E make_directory "${_pri_proj}"
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                        "${_pri_ctrl}" "${_pri_proj}/Microsoft.UI.Xaml.Controls.pri"
                COMMAND "${MPAPP_MAKEPRI}" new /pr "${_pri_proj}"
                        /cf "${CMAKE_SOURCE_DIR}/cmake/winui_priconfig.xml"
                        /of "${_pri_out}" /o
                DEPENDS "${_pri_ctrl}" "${CMAKE_SOURCE_DIR}/cmake/winui_priconfig.xml"
                COMMENT "MPAPP: makepri -> unpackaged resources.pri (Application map)"
                VERBATIM)
            add_custom_target(mpapp_winui_resources_pri DEPENDS "${_pri_out}")
        endif()
        add_dependencies(${target} mpapp_winui_resources_pri)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${_pri_out}" "$<TARGET_FILE_DIR:${target}>/resources.pri"
            COMMENT "MPAPP: staging resources.pri next to ${target}"
            VERBATIM)
    else()
        message(WARNING "MPAPP: makepri or WinUI controls PRI not found — "
            "unpackaged WinUI control resources may not resolve at runtime")
    endif()
endfunction()
