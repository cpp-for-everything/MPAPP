# cmake/MpappHardening.cmake
#
# Opt-in quality gates — sanitizers, coverage, clang-tidy, GCC -fanalyzer —
# ported from the MAUI++ Hardening.cmake matrix. A plain build carries NONE of
# these; they switch on only when a quality preset (see CMakePresets.json) sets
# the cache variables below:
#
#   MPAPP_SANITIZER  (STRING) none | asan | ubsan | asanubsan | tsan
#   MPAPP_COVERAGE   (STRING) none | gcov | llvm
#   MPAPP_CLANG_TIDY (BOOL)   run clang-tidy on first-party targets during build
#   MPAPP_FANALYZER  (BOOL)   run GCC -fanalyzer on first-party targets (report-only)
#
# Two application points:
#   * mpapp_init_hardening()  — global: sanitizer + coverage instrumentation is
#     applied to the WHOLE child build (incl. fetched Catch2) so instrumented
#     and library TUs stay ABI-consistent. Called once from MpappOptions.
#   * mpapp_apply_static_analysis(<target>) — per first-party target: clang-tidy
#     + -fanalyzer, scoped so FetchContent deps (Catch2, pugixml) aren't linted.
#     Called from mpapp_apply_warnings().
include_guard(GLOBAL)

set(MPAPP_SANITIZER "none" CACHE STRING "Sanitizer: none, asan, ubsan, asanubsan, tsan")
set_property(CACHE MPAPP_SANITIZER PROPERTY STRINGS none asan ubsan asanubsan tsan)
set(MPAPP_COVERAGE "none" CACHE STRING "Coverage instrumentation: none, gcov, llvm")
set_property(CACHE MPAPP_COVERAGE PROPERTY STRINGS none gcov llvm)
option(MPAPP_CLANG_TIDY "Run clang-tidy on first-party targets during build" OFF)
option(MPAPP_FANALYZER "Run GCC -fanalyzer on first-party targets (report-only)" OFF)

# Global sanitizer + coverage flags for the whole child build. add_compile_options
# / add_link_options here run at the directory scope of the includer (the root
# child-mode CMakeLists), so they reach src/, tests/, tools/, and the fetched
# Catch2 uniformly.
function(mpapp_init_hardening)
    # -- sanitizers (Clang/GCC only) -----------------------------------------
    if(NOT MPAPP_SANITIZER STREQUAL "none")
        if(MSVC)
            message(WARNING "MPAPP: MPAPP_SANITIZER='${MPAPP_SANITIZER}' not wired for MSVC; ignoring.")
        else()
            if(MPAPP_SANITIZER STREQUAL "asan")
                set(_s -fsanitize=address)            # ASan includes LeakSanitizer on Clang/GCC
            elseif(MPAPP_SANITIZER STREQUAL "asanubsan")
                set(_s -fsanitize=address,undefined)
            elseif(MPAPP_SANITIZER STREQUAL "ubsan")
                set(_s -fsanitize=undefined)
            elseif(MPAPP_SANITIZER STREQUAL "tsan")
                set(_s -fsanitize=thread)
            else()
                message(FATAL_ERROR "MPAPP: unknown MPAPP_SANITIZER '${MPAPP_SANITIZER}'")
            endif()
            add_compile_options(${_s} -g -fno-omit-frame-pointer -fno-sanitize-recover=all)
            add_link_options(${_s})
            message(STATUS "MPAPP hardening: sanitizer=${MPAPP_SANITIZER}")
        endif()
    endif()

    # -- coverage ------------------------------------------------------------
    if(NOT MPAPP_COVERAGE STREQUAL "none")
        if(MSVC)
            message(WARNING "MPAPP: MPAPP_COVERAGE='${MPAPP_COVERAGE}' not wired for MSVC; ignoring.")
        elseif(MPAPP_COVERAGE STREQUAL "gcov")
            add_compile_options(--coverage -O0 -g)
            add_link_options(--coverage)
            message(STATUS "MPAPP hardening: coverage=gcov")
        elseif(MPAPP_COVERAGE STREQUAL "llvm")
            add_compile_options(-fprofile-instr-generate -fcoverage-mapping -O0 -g)
            add_link_options(-fprofile-instr-generate -fcoverage-mapping)
            message(STATUS "MPAPP hardening: coverage=llvm")
        else()
            message(FATAL_ERROR "MPAPP: unknown MPAPP_COVERAGE '${MPAPP_COVERAGE}'")
        endif()
    endif()
endfunction()

# Per first-party target: clang-tidy + GCC -fanalyzer. No-op unless the
# corresponding option is on.
function(mpapp_apply_static_analysis target)
    if(MPAPP_CLANG_TIDY)
        if(NOT DEFINED MPAPP_CLANG_TIDY_COMMAND)
            find_program(MPAPP_CLANG_TIDY_EXE NAMES clang-tidy)
            if(MPAPP_CLANG_TIDY_EXE)
                # --quiet: suppress the "N warnings generated" noise. Findings
                # are report-only (no --warnings-as-errors) unless the caller
                # opts in via MPAPP_TIDY_WERROR.
                set(_cmd "${MPAPP_CLANG_TIDY_EXE};--quiet")
                if(MPAPP_TIDY_WERROR)
                    set(_cmd "${_cmd};--warnings-as-errors=*")
                endif()
                set(MPAPP_CLANG_TIDY_COMMAND "${_cmd}" CACHE INTERNAL "Resolved clang-tidy invocation")
            else()
                message(WARNING "MPAPP: MPAPP_CLANG_TIDY=ON but clang-tidy not found on PATH.")
                set(MPAPP_CLANG_TIDY_COMMAND "" CACHE INTERNAL "Resolved clang-tidy invocation")
            endif()
        endif()
        if(MPAPP_CLANG_TIDY_COMMAND)
            set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${MPAPP_CLANG_TIDY_COMMAND}")
        endif()
    endif()

    if(MPAPP_FANALYZER)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            # Report-only: GCC -fanalyzer over-fires once it inlines into libstdc++
            # internals, so findings are reviewed manually rather than promoted to
            # errors. "too-complex" bailouts are silenced.
            target_compile_options(${target} PRIVATE -fanalyzer -Wno-analyzer-too-complex)
        else()
            message(WARNING "MPAPP: MPAPP_FANALYZER set but compiler is not GCC; skipping on ${target}")
        endif()
    endif()
endfunction()
