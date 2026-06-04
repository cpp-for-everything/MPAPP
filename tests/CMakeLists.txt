# tests/CMakeLists.txt — portable unit tests (Catch2 + CTest).
#
# One mpapp_add_catch_test() per binary (see cmake/MpappTest.cmake): it links
# mpapp-core + Catch2, applies the strict warning policy, and registers with
# CTest. No per-test boilerplate.
include(MpappTest)
mpapp_enable_catch2()

mpapp_add_catch_test(smoke_test    SOURCES smoke_test.cpp)
mpapp_add_catch_test(executor_test SOURCES executor_test.cpp)

add_subdirectory(template_type_spike)

# Mock-handler unit tests for the P2 component surface (ADR-0008). Per ADR-0013
# the list is a glob — adding a widget's <widget>_test.cpp needs no edit here.
file(GLOB MPAPP_MOCK_TESTS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/mock_handlers/*_test.cpp")
mpapp_add_catch_test(mock_handlers_test SOURCES ${MPAPP_MOCK_TESTS})

# T-0010 hot-reload spike — Windows only.
if(WIN32)
    mpapp_add_catch_test(hot_reload_test SOURCES
        "${CMAKE_SOURCE_DIR}/vault/50_Tasks/_Archive/T-0010-hot-reload-spike/tests/hot_reload_test.cpp")
endif()
