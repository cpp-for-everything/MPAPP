# cmake/MpappWarnings.cmake
#
# Single definition of MPAPP's compiler-warning policy. Replaces the
# copy-pasted `$<COMPILE_LANG_AND_ID:...>` blocks that were duplicated across
# ~25 CMakeLists.txt files.
#
#   mpapp_apply_warnings(<target>)            strict: warnings are errors
#       GNU/Clang : -Wall -Wextra -Wpedantic -Werror
#       MSVC      : /W4 /WX /permissive- /Zc:__cplusplus
#
#   mpapp_apply_warnings(<target> RELAXED)    warnings on, not fatal
#       GNU/Clang : -Wall -Wextra -Wpedantic
#       MSVC      : /W3 /Zc:__cplusplus
#
# RELAXED is for targets that pull in third-party headers which don't survive
# -Werror / /WX cleanly (GTK4, WinUI 3 / C++-WinRT).
#
# Applying warnings to a target also opts it into the per-target static-analysis
# gates (clang-tidy / -fanalyzer) — these run on first-party targets only, never
# on fetched dependencies. See cmake/MpappHardening.cmake.
include_guard(GLOBAL)

include(MpappHardening)

function(mpapp_apply_warnings target)
    cmake_parse_arguments(A "RELAXED" "" "" ${ARGN})
    if(A_RELAXED)
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic>
            $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W3;/Zc:__cplusplus>
        )
    else()
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic;-Werror>
            $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4;/WX;/permissive-;/Zc:__cplusplus>
        )
    endif()
    mpapp_apply_static_analysis(${target})
endfunction()
