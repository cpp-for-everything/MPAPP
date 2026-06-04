# cmake/MpappHandlers.cmake
#
# Builds the per-platform native handler library ONCE, instead of having the
# first example subdirectory define it and the rest rely on add_subdirectory
# ordering (the old, fragile arrangement). Examples just link the target.
#
#   mpapp_add_handler_library(<platform>)     platform = windows | linux
#       Creates the static library mpapp-handlers-<platform> from
#       src/handlers/<platform>/*.cpp (globbed per ADR-0013) with the right
#       native SDK wiring. Idempotent: a no-op if the target already exists.
#
# The handlers are NOT compiled into mpapp-core because they pull in heavy
# native SDKs (WindowsAppSDK / C++/WinRT projection, GTK4 / WebKitGTK).
include_guard(GLOBAL)

function(mpapp_add_handler_library platform)
    if(TARGET mpapp-handlers-${platform})
        return()
    endif()

    file(GLOB _handler_srcs CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/handlers/${platform}/*.cpp")

    if(platform STREQUAL "windows")
        include(WindowsAppSDK)
        mpapp_install_windows_app_sdk()

        add_library(mpapp-handlers-windows STATIC ${_handler_srcs})
        target_include_directories(mpapp-handlers-windows PUBLIC
            "${CMAKE_SOURCE_DIR}/include")
        # application_handler.cpp pulls in <MddBootstrap.h>; its headers live
        # in the Foundation NuGet. SYSTEM so /WX in consumers ignores them.
        target_include_directories(mpapp-handlers-windows SYSTEM PRIVATE
            "${MPAPP_WINAPPSDK_FOUNDATION_DIR}/include")
        target_compile_features(mpapp-handlers-windows PUBLIC cxx_std_23)
        target_compile_definitions(mpapp-handlers-windows PUBLIC
            WINRT_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
        # Generate the cppwinrt projection and propagate its include dir
        # (the helper sets it PUBLIC so consumers pick it up).
        mpapp_generate_winrt_projection(mpapp-handlers-windows MPAPP_WINRT_PROJECTION_DIR)
        # WinUI 3 headers emit deprecation warnings under /W4 /WX — loosen.
        target_compile_options(mpapp-handlers-windows PRIVATE /W3 /Zc:__cplusplus)
        target_link_libraries(mpapp-handlers-windows PUBLIC mpapp-core)

    elseif(platform STREQUAL "linux")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
        # WebKitGTK (LGPL, dynamically linked) drives the web_view handlers.
        # Require the modern webkitgtk-6.0 binding (webkit2gtk-4.1 is GTK3-only).
        pkg_check_modules(WEBKIT_GTK QUIET IMPORTED_TARGET webkitgtk-6.0)

        add_library(mpapp-handlers-linux STATIC ${_handler_srcs})
        target_include_directories(mpapp-handlers-linux PUBLIC
            "${CMAKE_SOURCE_DIR}/include")
        target_compile_features(mpapp-handlers-linux PUBLIC cxx_std_23)
        target_link_libraries(mpapp-handlers-linux
            PUBLIC mpapp-core PkgConfig::GTK4)
        if(WEBKIT_GTK_FOUND)
            target_link_libraries(mpapp-handlers-linux PUBLIC PkgConfig::WEBKIT_GTK)
            target_compile_definitions(mpapp-handlers-linux PRIVATE MPAPP_HAS_WEBKITGTK=1)
        else()
            message(WARNING "MPAPP: WebKitGTK not found — web_view handler is a no-op on Linux")
        endif()
        # GTK4 headers don't survive -Wpedantic -Werror cleanly.
        target_compile_options(mpapp-handlers-linux PRIVATE -Wall -Wextra)

    else()
        message(FATAL_ERROR "mpapp_add_handler_library: unknown platform '${platform}' (expected windows|linux)")
    endif()
endfunction()
