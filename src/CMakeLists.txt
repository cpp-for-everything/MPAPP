# src/CMakeLists.txt — the portable MPAPP core library.
#
# mpapp-core is platform-neutral: the native UI handlers (WinUI 3, GTK4, ...)
# live in their own mpapp-handlers-<platform> libraries (see
# cmake/MpappHandlers.cmake) because they pull in heavy native SDKs. The one
# bit of conditional logic that can't be a plain source list — selecting the
# 2D graphics backend — is the single mpapp_select_graphics_backend() call.
include(MpappWarnings)
include(MpappGraphicsBackend)

add_library(mpapp-core STATIC
    mpapp.cpp
    executor/mock.cpp
    detail/graphics/canvas.cpp
    detail/graphics/shape_renderer.cpp
)

# Hot-reload runtime is per-OS (dlopen / LoadLibrary based).
if(WIN32)
    target_sources(mpapp-core PRIVATE hot_reload/windows.cpp)
elseif(UNIX AND NOT APPLE)
    target_sources(mpapp-core PRIVATE hot_reload/linux.cpp)
endif()

target_include_directories(mpapp-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# The Linux hot-reload runtime dlopen's recompiled user code — link libdl.
if(UNIX AND NOT APPLE)
    target_link_libraries(mpapp-core PUBLIC ${CMAKE_DL_LIBS})
endif()

target_compile_features(mpapp-core PUBLIC cxx_std_23)

# Add the selected backend .cpp + its includes/libs/defines (cairo|stub|skia).
mpapp_select_graphics_backend(mpapp-core)

mpapp_apply_warnings(mpapp-core)
