# SPDX-License-Identifier: Apache-2.0
# T-0011 — Android example native CMakeLists. Invoked by Gradle's
# externalNativeBuild with `ANDROID_PLATFORM`, `ANDROID_ABI`, etc.
# pre-set.
#
# Per ADR-0013 the handler source list is a glob — adding a new
# widget's *_handler.cpp doesn't require touching this file.

cmake_minimum_required(VERSION 3.28)

project(android_hello CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MPAPP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../..")

file(GLOB MPAPP_ANDROID_HANDLERS CONFIGURE_DEPENDS
    ${MPAPP_ROOT}/src/handlers/android/*.cpp
)

add_library(android_hello SHARED
    native_main.cpp
    ${MPAPP_ROOT}/src/mpapp.cpp
    ${MPAPP_ANDROID_HANDLERS}
)

target_include_directories(android_hello PRIVATE "${MPAPP_ROOT}/include")
target_link_libraries(android_hello PRIVATE log android)
