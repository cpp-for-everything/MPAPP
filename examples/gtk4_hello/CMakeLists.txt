# SPDX-License-Identifier: Apache-2.0
# T-0011 — GTK4 hello-window example, rewritten against the MPAPP
# app-shell abstraction. Compiles + runs only on UNIX (non-Apple).
#
# Per ADR-0013 the handler source list is a glob — adding a new
# widget's *_handler.cpp doesn't require touching this file.

if(NOT (UNIX AND NOT APPLE))
    return()
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
# WebKitGTK (LGPL, dynamically linked per Rule 9 / RFC-0001) — drives
# the Linux web_view / hybrid_web_view handlers. Require the modern
# webkitgtk-6.0 binding: the legacy webkit2gtk-4.1 is GTK3-only and
# its headers can't be combined with a GTK4 build.
pkg_check_modules(WEBKIT_GTK QUIET IMPORTED_TARGET webkitgtk-6.0)

# MPAPP Linux handler set — single small static library, mirrors the
# Windows handler set layout (examples/windows_button_spike/CMakeLists).
file(GLOB MPAPP_LINUX_HANDLERS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/handlers/linux/*.cpp
)

add_library(mpapp-handlers-linux STATIC
    ${MPAPP_LINUX_HANDLERS}
)

target_include_directories(mpapp-handlers-linux PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(mpapp-handlers-linux PUBLIC cxx_std_23)

target_link_libraries(mpapp-handlers-linux
    PUBLIC mpapp-core
    PUBLIC PkgConfig::GTK4
)
if(WEBKIT_GTK_FOUND)
    target_link_libraries(mpapp-handlers-linux PUBLIC PkgConfig::WEBKIT_GTK)
    target_compile_definitions(mpapp-handlers-linux PRIVATE MPAPP_HAS_WEBKITGTK=1)
else()
    message(WARNING "MPAPP: WebKitGTK not found — web_view handler will be a no-op on Linux")
endif()

# GTK4 headers don't survive -Wpedantic -Werror cleanly; loosen on this
# library only.
target_compile_options(mpapp-handlers-linux PRIVATE
    -Wall -Wextra
)

# The example exe.
add_executable(gtk4_hello main.cpp)
target_link_libraries(gtk4_hello PRIVATE
    mpapp-core
    mpapp-handlers-linux
)
target_compile_features(gtk4_hello PRIVATE cxx_std_23)
target_compile_options(gtk4_hello PRIVATE
    -Wall -Wextra -Wpedantic
)
