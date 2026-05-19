# SPDX-License-Identifier: Apache-2.0
# T-0011 — GTK4 hello-window example, rewritten against the MPAPP
# app-shell abstraction. Compiles + runs only on UNIX (non-Apple).

if(NOT (UNIX AND NOT APPLE))
    return()
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

# MPAPP Linux handler set — single small static library, mirrors the
# Windows handler set layout (examples/windows_button_spike/CMakeLists).
add_library(mpapp-handlers-linux STATIC
    ${CMAKE_SOURCE_DIR}/src/handlers/linux/application_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/linux/window_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/linux/stack_layout_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/linux/button_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/linux/label_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/linux/entry_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/linux/switch_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/linux/check_box_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/handlers/linux/radio_button_handler.cpp
)

target_include_directories(mpapp-handlers-linux PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(mpapp-handlers-linux PUBLIC cxx_std_23)

target_link_libraries(mpapp-handlers-linux
    PUBLIC mpapp-core
    PUBLIC PkgConfig::GTK4
)

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
