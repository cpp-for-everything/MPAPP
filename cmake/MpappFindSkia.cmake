# SPDX-License-Identifier: Apache-2.0
# Part of MPAPP. T-0030 — locate or auto-download a Skia install.
#
# Skia is opt-in (per ADR-0015). Once selected via
# `-DMPAPP_GRAPHICS_BACKEND=skia`, this helper does the heavy lifting:
#
#   1. If MPAPP_SKIA_PREFIX is set, probe that prefix in either of two
#      layouts — vcpkg's `unofficial-skia` CMake config package, or a
#      HumbleUI/SkiaBuild / JetBrains/skia-pack prebuilt drop. Useful
#      for offline builds, internal mirrors, or pinning to a specific
#      vcpkg revision.
#
#   2. Otherwise, **download the pinned community prebuilt for the
#      target platform** via FetchContent. Cached in
#      `<build>/_deps/mpapp_skia_prebuilt-src/`, verified by SHA-256,
#      and re-used across reconfigure. First-time cost: 40-70 MB
#      download + ~10 s extract. No vcpkg required.
#
# Either path produces the same imported target `unofficial::skia::skia`
# (INTERFACE-imported when assembled from a prebuilt; real CONFIG target
# when vcpkg won). The rest of the MPAPP build links one target and
# doesn't care which path served it.
#
# Inputs:
#   MPAPP_SKIA_PREFIX (optional) — install root for the offline /
#                                  override path. Probed first if set.
#
# Outputs (set in the caller's scope — this is a macro, not a function):
#   MPAPP_SKIA_FOUND   — ON when a usable install is located.
#   MPAPP_SKIA_LAYOUT  — "vcpkg" | "prebuilt" | "fetched" | "none".
#   unofficial::skia::skia  imported target — includes + .a/.lib list +
#                                             SK_* compile definitions.
#
# The prebuilt drops come from https://github.com/HumbleUI/SkiaBuild —
# a small community CI repo that publishes per-arch Skia static libs +
# headers + a `defines.cmake` generated from the actual ninja invocation
# (so the headers compile with the same SK_* flags the .a/.lib files
# were built with). JetBrains/skia-pack ships the same layout — either
# zip drops in identically.

# Pinned version. Bumping = deliberate change: re-fetch each platform
# from the HumbleUI release page, recompute SHA256s, update the table
# below.
set(MPAPP_SKIA_PREBUILT_VERSION "m143-da51f0d60e-4"
    CACHE STRING "HumbleUI/SkiaBuild release tag for the auto-fetched prebuilt.")
set(MPAPP_SKIA_PREBUILT_BASE_URL
    "https://github.com/HumbleUI/SkiaBuild/releases/download/${MPAPP_SKIA_PREBUILT_VERSION}"
    CACHE STRING "Base URL for the auto-fetched Skia prebuilt zips.")

# Per-target SHA-256 hashes for the Release zips. CMake doesn't have
# proper maps; flat _MPAPP_SKIA_SHA256_<platform>-<arch> variables work
# fine and the variable-name dispatch is contained in the selector
# function below. Hashes computed from the m143-da51f0d60e-4 release
# at https://github.com/HumbleUI/SkiaBuild/releases/tag/m143-da51f0d60e-4 .
set(_MPAPP_SKIA_SHA256_android-arm64
    "5e82b29f132d9265d25f947ac62f83d2a7d524195d568839547be4427b0a9855")
set(_MPAPP_SKIA_SHA256_android-x64
    "aee1cfdb12e0e004f5d3f4d98e970e9e7755360372945387bf186752dd99cd5d")
set(_MPAPP_SKIA_SHA256_linux-x64
    "06a0a7390d82e33c4998c5482c580d58bb692606ea20f061a37912d93ad5106f")
set(_MPAPP_SKIA_SHA256_macos-arm64
    "d34aa6fadf641987046ab7bb48839060826fa439964c8fb741bf98fbb240ff37")
set(_MPAPP_SKIA_SHA256_macos-x64
    "64bf1636ee32432c015dc25a7d796b3f46acd14606473af5d3b5593fa81b724c")
set(_MPAPP_SKIA_SHA256_windows-x64
    "c828ff9458cfae12bf695a0ee0b37fda3da8d5584f36b96f2df74a06730dd174")
# Add windows-arm64 / linux-arm64 rows when those become MPAPP targets;
# HumbleUI ships them in the same release tag.

# Map (CMAKE_SYSTEM_NAME, target arch) → (platform-key, arch-key) used
# in the URL + hash lookup. Returns empty url when the current platform
# isn't in the table (caller falls back to stub).
function(_mpapp_skia_select_release out_platform out_arch out_url out_hash)
    set(_platform "")
    set(_arch "")

    if(CMAKE_SYSTEM_NAME STREQUAL "Android")
        set(_platform "android")
        if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
            set(_arch "arm64")
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
            set(_arch "x64")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(_platform "linux")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            set(_arch "arm64")
        else()
            set(_arch "x64")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(_platform "windows")
        # CMAKE_SYSTEM_PROCESSOR on Windows is "AMD64" for x64 and
        # "ARM64" for Surface Pro X-style hosts. Default to x64 when
        # neither matches (older CMakes left it empty).
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
            set(_arch "arm64")
        else()
            set(_arch "x64")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(_platform "macos")
        # CMAKE_OSX_ARCHITECTURES wins over the host arch when the user
        # is doing universal / cross builds. Pick the first listed.
        if(CMAKE_OSX_ARCHITECTURES)
            list(GET CMAKE_OSX_ARCHITECTURES 0 _osx_arch)
        else()
            set(_osx_arch "${CMAKE_SYSTEM_PROCESSOR}")
        endif()
        if(_osx_arch MATCHES "arm64|aarch64")
            set(_arch "arm64")
        else()
            set(_arch "x64")
        endif()
    endif()

    if(_platform STREQUAL "" OR _arch STREQUAL "")
        return()
    endif()

    set(_key "${_platform}-${_arch}")
    if(NOT DEFINED _MPAPP_SKIA_SHA256_${_key})
        return()
    endif()

    set(${out_platform} "${_platform}" PARENT_SCOPE)
    set(${out_arch}     "${_arch}"     PARENT_SCOPE)
    set(${out_url}
        "${MPAPP_SKIA_PREBUILT_BASE_URL}/Skia-${MPAPP_SKIA_PREBUILT_VERSION}-${_platform}-Release-${_arch}.zip"
        PARENT_SCOPE)
    set(${out_hash} "${_MPAPP_SKIA_SHA256_${_key}}" PARENT_SCOPE)
endfunction()

# Build the imported `unofficial::skia::skia` target from a prebuilt
# tree rooted at ${prefix}. Recognizes both Linux/Android style
# (`out/Release-<arch>/libskia.a`) and Windows MSVC style
# (`out/Release-<arch>/skia.lib`) — they share the same out-dir +
# defines.cmake structure, just different file extensions.
#
# Macro (not function) so the imported target lands in the caller's
# directory scope and stays visible to the rest of the build.
macro(_mpapp_skia_apply_prebuilt_layout _mpapp_skia_prefix)
    set(_mpapp_skia_libdir "")
    set(_mpapp_skia_main "")
    if(IS_DIRECTORY "${_mpapp_skia_prefix}/out")
        file(GLOB _mpapp_skia_out_dirs
             RELATIVE "${_mpapp_skia_prefix}/out"
             "${_mpapp_skia_prefix}/out/*")
        foreach(_d IN LISTS _mpapp_skia_out_dirs)
            if(EXISTS "${_mpapp_skia_prefix}/out/${_d}/defines.cmake")
                if(EXISTS "${_mpapp_skia_prefix}/out/${_d}/libskia.a")
                    set(_mpapp_skia_libdir "${_mpapp_skia_prefix}/out/${_d}")
                    set(_mpapp_skia_main "${_mpapp_skia_libdir}/libskia.a")
                    break()
                elseif(EXISTS "${_mpapp_skia_prefix}/out/${_d}/skia.lib")
                    set(_mpapp_skia_libdir "${_mpapp_skia_prefix}/out/${_d}")
                    set(_mpapp_skia_main "${_mpapp_skia_libdir}/skia.lib")
                    break()
                endif()
            endif()
        endforeach()
    endif()

    if(NOT _mpapp_skia_libdir STREQUAL "")
        # Enumerate every static archive in the out/ dir, both .a and
        # .lib. Put the main skia library first so lld/link.exe satisfy
        # transitive-dep symbols requested by it without needing a
        # second pass.
        file(GLOB _mpapp_skia_libs
             "${_mpapp_skia_libdir}/*.a"
             "${_mpapp_skia_libdir}/*.lib")
        list(REMOVE_ITEM _mpapp_skia_libs "${_mpapp_skia_main}")
        list(PREPEND _mpapp_skia_libs "${_mpapp_skia_main}")

        # Pull the SK_* defines out of the HumbleUI-generated
        # defines.cmake and attach them to the imported target. The
        # file looks like `add_definitions(-DFOO -DBAR=BAZ ...)` — a
        # regex extracts the tokens so we can stash them as
        # INTERFACE_COMPILE_DEFINITIONS (scoped to consumers of the
        # target) rather than spraying them project-wide.
        file(READ "${_mpapp_skia_libdir}/defines.cmake"
             _mpapp_skia_defs_text)
        string(REGEX MATCHALL
            "-D[A-Za-z_][A-Za-z0-9_]*(=[^ \n\t)]+)?"
            _mpapp_skia_defs_list "${_mpapp_skia_defs_text}")
        list(TRANSFORM _mpapp_skia_defs_list REPLACE "^-D" "")

        # Drop defines that collide with what MPAPP consumer code
        # (especially src/hot_reload/windows.cpp + anything pulling
        # in <windows.h>) sets for itself. Skia's static libs are
        # already compiled with these on; what matters at the
        # consumer side is just that <Windows.h> + Skia's public
        # headers see consistent values. We keep our own and drop
        # Skia's — otherwise MSVC's /WX promotes the C4005
        # macro-redefinition warning into a build break. These
        # tokens are convenience defines (don't change Skia's ABI
        # or its public-header behavior), not the SK_* feature
        # flags that gate API surface.
        set(_mpapp_skia_drops
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            _CRT_SECURE_NO_WARNINGS
            _CRT_NONSTDC_NO_DEPRECATE
            UNICODE
            _UNICODE
            NDEBUG)
        foreach(_drop IN LISTS _mpapp_skia_drops)
            list(FILTER _mpapp_skia_defs_list EXCLUDE REGEX
                 "^${_drop}(=.*)?$")
        endforeach()

        # Platform system libs the prebuilt expects on the link line.
        # HumbleUI's Linux build sets `skia_use_system_freetype2=true`
        # — so freetype/fontconfig must come from the system. The other
        # entries mirror what vcpkg's `unofficial-skia` Linux config
        # passes (modulo the deps HumbleUI bundles into the zip as
        # .a files: png, jpeg, webp, expat, zlib, harfbuzz, icu).
        #
        # Windows + Android + macOS prebuilts bundle everything they
        # need from third_party/, so only the platform's own
        # system-provided libraries are added here.
        set(_mpapp_skia_syslibs "")
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            list(APPEND _mpapp_skia_syslibs
                freetype fontconfig
                GL EGL GLESv2 # SK_GL / SK_GANESH refs even when unused
                m dl pthread)
        elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
            list(APPEND _mpapp_skia_syslibs
                EGL GLESv2 # SK_GL paths
                log android)
        elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
            list(APPEND _mpapp_skia_syslibs
                user32 gdi32 opengl32 winmm fontsub usp10
                ole32 oleaut32 advapi32 # GDI / DirectWrite / D3D plumbing
                d3d12 d3dcompiler dxgi)
        elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            # macOS uses framework linkage; CMake expects "-framework Foo"
            # tokens directly in INTERFACE_LINK_LIBRARIES.
            list(APPEND _mpapp_skia_syslibs
                "-framework CoreFoundation"
                "-framework CoreGraphics"
                "-framework CoreText"
                "-framework CoreServices"
                "-framework Foundation"
                "-framework Metal"
                "-framework MetalKit")
        endif()

        add_library(unofficial::skia::skia INTERFACE IMPORTED)
        set_target_properties(unofficial::skia::skia PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_mpapp_skia_prefix}"
            INTERFACE_LINK_LIBRARIES       "${_mpapp_skia_libs};${_mpapp_skia_syslibs}"
            INTERFACE_COMPILE_DEFINITIONS  "${_mpapp_skia_defs_list}"
        )
        set(MPAPP_SKIA_FOUND ON)
    endif()
endmacro()

macro(mpapp_find_skia)
    set(MPAPP_SKIA_FOUND OFF)
    set(MPAPP_SKIA_LAYOUT "none")

    # Path 1: explicit MPAPP_SKIA_PREFIX — try vcpkg config first,
    # fall back to the prebuilt layout at that prefix.
    if(DEFINED MPAPP_SKIA_PREFIX)
        list(APPEND CMAKE_PREFIX_PATH "${MPAPP_SKIA_PREFIX}")
        find_package(unofficial-skia CONFIG QUIET)
        if(TARGET unofficial::skia::skia)
            set(MPAPP_SKIA_FOUND ON)
            set(MPAPP_SKIA_LAYOUT "vcpkg")
        else()
            _mpapp_skia_apply_prebuilt_layout("${MPAPP_SKIA_PREFIX}")
            if(MPAPP_SKIA_FOUND)
                set(MPAPP_SKIA_LAYOUT "prebuilt")
            endif()
        endif()
    endif()

    # Path 2: auto-download the pinned prebuilt for the current target.
    # Windows /MT vs /MD compatibility check first. HumbleUI's Windows
    # Skia prebuilt is compiled with /MT (static CRT) because that's
    # Skia's `is_official_build=true` Windows default. Linking /MT
    # static libs into a /MD consumer triggers MSVC LNK2038
    # ("RuntimeLibrary mismatch") and breaks the build. Most MPAPP
    # consumers — WinUI 3 / WindowsAppSDK / WebView2 apps — are /MD.
    # So on Windows, only auto-fetch when the project has explicitly
    # opted into /MT via CMAKE_MSVC_RUNTIME_LIBRARY; otherwise emit a
    # clear message + skip the fetch (caller falls through to stub).
    set(_mpapp_skia_msvc_crt_ok TRUE)
    if(MSVC AND CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MPAPP_SKIA_FOUND)
        if(NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY
            OR NOT CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "^MultiThreaded(Debug)?$")
            set(_mpapp_skia_msvc_crt_ok FALSE)
            message(STATUS
                "MPAPP Skia auto-fetch skipped on Windows: the pinned "
                "HumbleUI/SkiaBuild prebuilt is /MT (static CRT), but "
                "this project uses /MD (dynamic CRT, the default and "
                "what WinUI 3 / WindowsAppSDK require). To use Skia "
                "on Windows pass one of:\n"
                "  -DMPAPP_SKIA_PREFIX=<vcpkg-installed-dir>  (vcpkg builds /MD by default)\n"
                "  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>  (switch project to /MT; breaks WinAppSDK)\n"
                "Falling through to stub backend in the meantime.")
        endif()
    endif()

    if(NOT MPAPP_SKIA_FOUND AND _mpapp_skia_msvc_crt_ok)
        _mpapp_skia_select_release(_mpapp_skia_platform _mpapp_skia_arch
                                   _mpapp_skia_url _mpapp_skia_hash)
        if(DEFINED _mpapp_skia_url AND NOT _mpapp_skia_url STREQUAL "")
            include(FetchContent)
            # DOWNLOAD_EXTRACT_TIMESTAMP suppresses CMP0135's warning
            # about archive timestamps — we want extracted files to
            # carry the download timestamp so subsequent configures
            # don't see false-positive "newer than" mtime drift.
            FetchContent_Declare(mpapp_skia_prebuilt
                URL                       "${_mpapp_skia_url}"
                URL_HASH                  "SHA256=${_mpapp_skia_hash}"
                DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            )
            message(STATUS
                "MPAPP fetching Skia prebuilt: ${_mpapp_skia_platform}-${_mpapp_skia_arch} "
                "(${MPAPP_SKIA_PREBUILT_VERSION})")
            FetchContent_MakeAvailable(mpapp_skia_prebuilt)
            _mpapp_skia_apply_prebuilt_layout("${mpapp_skia_prebuilt_SOURCE_DIR}")
            if(MPAPP_SKIA_FOUND)
                set(MPAPP_SKIA_LAYOUT "fetched")
            endif()
        endif()
    endif()
endmacro()
