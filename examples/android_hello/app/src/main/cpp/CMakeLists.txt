# SPDX-License-Identifier: Apache-2.0
# T-0011 — Android example native CMakeLists. Invoked by Gradle's
# externalNativeBuild with `ANDROID_PLATFORM`, `ANDROID_ABI`, etc.
# pre-set.

cmake_minimum_required(VERSION 3.28)

project(android_hello CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MPAPP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../..")

add_library(android_hello SHARED
    native_main.cpp
    ${MPAPP_ROOT}/src/handlers/android/jni_bridge.cpp
    ${MPAPP_ROOT}/src/handlers/android/window_handler.cpp
    ${MPAPP_ROOT}/src/handlers/android/stack_layout_handler.cpp
    ${MPAPP_ROOT}/src/handlers/android/button_handler.cpp
    ${MPAPP_ROOT}/src/handlers/android/label_handler.cpp
    ${MPAPP_ROOT}/src/handlers/android/entry_handler.cpp
    ${MPAPP_ROOT}/src/mpapp.cpp
)

target_include_directories(android_hello PRIVATE "${MPAPP_ROOT}/include")
target_link_libraries(android_hello PRIVATE log android)
