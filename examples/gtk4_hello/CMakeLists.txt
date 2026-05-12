# SPDX-License-Identifier: Apache-2.0
# T-0007 — WSLg GTK4 hello-window example.
#
# Linux-only GTK4 smoke target. Used to validate the Linux dev loop from
# a Windows host: build under WSL2 Ubuntu (or cross-build from the host
# via the linux-x64 toolchain) and run the binary so the window surfaces
# on the Windows desktop through WSLg.
#
# The example is gated on UNIX AND NOT APPLE — Linux/BSD only. macOS is
# excluded because GTK4 there is not the native toolkit.

if(NOT (UNIX AND NOT APPLE))
    return()
endif()

# Enable C so we can compile main.c. The project file lists CXX only, so
# we have to add C locally for this target.
enable_language(C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

add_executable(gtk4_hello main.c)

target_link_libraries(gtk4_hello PRIVATE PkgConfig::GTK4)

target_compile_options(gtk4_hello PRIVATE
    $<$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
)

# GTK4 headers require C11 (stdatomic.h, _Generic).
target_compile_features(gtk4_hello PRIVATE c_std_11)
