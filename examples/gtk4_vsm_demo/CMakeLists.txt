# SPDX-License-Identifier: Apache-2.0
# RFC-0006 Visual State Manager demo — GTK4 / Linux.
#
# Mirrors examples/gtk4_hello/CMakeLists.txt — reuses the already-built
# mpapp-handlers-linux library declared by that sibling target via
# CMake's PROPAGATE rules (the root examples/CMakeLists.txt builds the
# handler lib once + every example links against it).

if(NOT (UNIX AND NOT APPLE))
    return()
endif()

add_executable(gtk4_vsm_demo main.cpp)
target_link_libraries(gtk4_vsm_demo PRIVATE
    mpapp-core
    mpapp-handlers-linux
)
target_compile_features(gtk4_vsm_demo PRIVATE cxx_std_23)
target_compile_options(gtk4_vsm_demo PRIVATE
    -Wall -Wextra -Wpedantic
)
